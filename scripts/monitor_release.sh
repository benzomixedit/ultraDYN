#!/usr/bin/env bash
set -euo pipefail

# Monitor GitHub Actions release workflow and retry by re-tagging v1.0.0 until success
# Checks status every 60 seconds and fetches logs on failure (if token available).

extract_token() {
	git remote get-url origin | python3 - "$@" <<'PY'
import re, sys
u=sys.stdin.read().strip()
m=re.search(r"x-access-token:([^@]+)@", u)
print(m.group(1) if m else "")
PY
}

extract_owner_repo() {
	git remote get-url origin | python3 - <<'PY'
import re, sys
u=sys.stdin.read().strip()
m=re.search(r"github\.com/([^/]+)/([^/.]+)", u)
if not m:
	print(" ")
else:
	print(m.group(1), m.group(2))
PY
}

TOKEN="${GITHUB_TOKEN:-}"
if [[ -z "$TOKEN" ]]; then
	TOKEN="$(extract_token)"
fi

read OWNER REPO < <(extract_owner_repo)
if [[ -z "${OWNER:-}" || -z "${REPO:-}" ]]; then
	echo "[monitor] ERROR: Could not parse owner/repo from remote URL" >&2
	exit 1
fi

if [[ -n "$TOKEN" ]]; then
	AUTH_HEADER=( -H "Authorization: token $TOKEN" )
	echo "[monitor] Using authenticated GitHub API"
else
	AUTH_HEADER=()
	echo "[monitor] No token found; using unauthenticated GitHub API (rate-limited, logs may be unavailable)"
fi

echo "[monitor] Monitoring $OWNER/$REPO release workflow every 60s..."

# Find workflow id for release.yml
WF_ID="$(curl -fsSL "https://api.github.com/repos/$OWNER/$REPO/actions/workflows" "${AUTH_HEADER[@]}" | python3 - <<'PY'
import sys, json
w=json.load(sys.stdin)
for it in w.get('workflows', []):
	if it.get('path') == '.github/workflows/release.yml':
		print(it.get('id'))
		break
PY
)" || true
if [[ -z "$WF_ID" ]]; then
	echo "[monitor] ERROR: Could not find workflow id for .github/workflows/release.yml" >&2
	exit 1
fi

last_run_id=""

while :; do
	# Get latest run id
	run_id="$(curl -fsSL "https://api.github.com/repos/$OWNER/$REPO/actions/workflows/$WF_ID/runs?per_page=1" "${AUTH_HEADER[@]}" | python3 - <<'PY'
import sys, json
j=json.load(sys.stdin)
runs=j.get('runs', [])
print(runs[0]['id'] if runs else "")
PY
)" || true
	if [[ -z "$run_id" ]]; then
		echo "[monitor] No runs found yet; sleeping..."
		sleep 60
		continue
	fi
	if [[ "$run_id" != "$last_run_id" ]]; then
		echo "[monitor] Tracking run $run_id"
		last_run_id="$run_id"
	fi

	# Check status and conclusion
	read -r status conclusion < <(curl -fsSL "https://api.github.com/repos/$OWNER/$REPO/actions/runs/$run_id" "${AUTH_HEADER[@]}" | python3 - <<'PY'
import sys, json
j=json.load(sys.stdin)
print(j.get('status',''), j.get('conclusion',''))
PY
)
	echo "[monitor] Status: $status, Conclusion: ${conclusion:-}"
	if [[ "$status" == "completed" ]]; then
		if [[ "${conclusion:-}" == "success" ]]; then
			echo "[monitor] Release workflow succeeded. Exiting monitor."
			exit 0
		fi
		# Failed or cancelled: fetch logs and retry
		echo "[monitor] Workflow failed; fetching job logs..."
		if [[ -n "$TOKEN" ]]; then
			jobs_json="$(curl -fsSL "https://api.github.com/repos/$OWNER/$REPO/actions/runs/$run_id/jobs" "${AUTH_HEADER[@]}")"
			echo "$jobs_json" | python3 - <<'PY'
import sys, json
j=json.load(sys.stdin)
for job in j.get('jobs', []):
	print(f"JOB {job['id']} {job['name']} {job.get('conclusion','')}")
PY
			for job_id in $(echo "$jobs_json" | python3 - <<'PY'
import sys, json
j=json.load(sys.stdin)
print(' '.join(str(job['id']) for job in j.get('jobs', [])))
PY
			); do
				url="$(curl -I -sS "https://api.github.com/repos/$OWNER/$REPO/actions/jobs/$job_id/logs" "${AUTH_HEADER[@]}" | awk -F': ' '/^location: /{print $2}' | tr -d '\r')"
				if [[ -n "$url" ]]; then
					curl -fsSL "$url" -o "logs_${job_id}.txt" || true
				fi
			done
			grep -iE "error:|failed|fatal|traceback" -n logs_*.txt | head -n 50 || true
		else
			echo "[monitor] No token; skipping detailed logs download."
		fi
		echo "[monitor] Retagging v1.0.0 to retry..."
		git tag -f v1.0.0
		git push --delete origin v1.0.0 || true
		git push origin v1.0.0 -f
		# After retry, wait a minute before next check
		sleep 60
		continue
	fi
	# Not completed yet; wait a minute and re-check
	sleep 60
done