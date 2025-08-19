#!/usr/bin/env python3
import os, re, sys, json, subprocess, time, urllib.request

def get_remote_url() -> str:
    return subprocess.check_output(["git", "remote", "get-url", "origin"], text=True).strip()

def extract_owner_repo(url: str):
    m = re.search(r"github\.com/([^/]+)/([^/.]+)", url)
    if not m:
        return None, None
    return m.group(1), m.group(2)

def extract_token(url: str):
    m = re.search(r"x-access-token:([^@]+)@", url)
    return m.group(1) if m else None

def api_get(url: str, token: str | None):
    req = urllib.request.Request(url)
    if token:
        req.add_header("Authorization", f"token {token}")
    with urllib.request.urlopen(req) as resp:
        return json.load(resp)

def api_head(url: str, token: str | None):
    req = urllib.request.Request(url, method="HEAD")
    if token:
        req.add_header("Authorization", f"token {token}")
    with urllib.request.urlopen(req) as resp:
        return dict(resp.headers)

def download(url: str, dest: str):
    with urllib.request.urlopen(url) as resp, open(dest, "wb") as f:
        f.write(resp.read())

def main():
    remote = get_remote_url()
    owner, repo = extract_owner_repo(remote)
    if not owner or not repo:
        print("ERR: cannot parse owner/repo", file=sys.stderr)
        sys.exit(1)
    token = extract_token(remote)
    try:
        workflows = api_get(f"https://api.github.com/repos/{owner}/{repo}/actions/workflows", token)
    except Exception as e:
        print(f"ERR: list workflows failed: {e}", file=sys.stderr)
        sys.exit(1)
    wf_id = None
    for wf in workflows.get("workflows", []):
        if wf.get("path") == ".github/workflows/release.yml":
            wf_id = wf.get("id")
            break
    if not wf_id:
        print("ERR: release.yml workflow not found", file=sys.stderr)
        sys.exit(1)
    runs = api_get(f"https://api.github.com/repos/{owner}/{repo}/actions/workflows/{wf_id}/runs?per_page=5", token)
    run = (runs.get("runs") or [None])[0]
    if not run:
        print("No runs found")
        return
    print(f"run_id={run['id']} status={run['status']} conclusion={run['conclusion']} url={run['html_url']}")
    run_detail = api_get(f"https://api.github.com/repos/{owner}/{repo}/actions/runs/{run['id']}", token)
    if run_detail.get("status") == "completed" and run_detail.get("conclusion") != "success":
        jobs = api_get(f"https://api.github.com/repos/{owner}/{repo}/actions/runs/{run['id']}/jobs", token)
        for job in jobs.get("jobs", []):
            print(f"JOB {job['id']} {job['name']} {job.get('conclusion','')}")
        if token:
            for job in jobs.get("jobs", []):
                # Get redirected logs URL
                try:
                    headers = api_head(f"https://api.github.com/repos/{owner}/{repo}/actions/jobs/{job['id']}/logs", token)
                    loc = headers.get("location")
                    if loc:
                        dest = f"logs_{job['id']}.txt"
                        download(loc, dest)
                        print(f"Downloaded {dest}")
                except Exception as e:
                    print(f"WARN: failed to download logs for job {job['id']}: {e}")
        else:
            print("Note: no token; cannot download job logs")
    else:
        print("Latest run is in-progress or succeeded")

if __name__ == "__main__":
    main()