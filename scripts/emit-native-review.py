#!/usr/bin/env python3
"""Small, integrity-checked native screenshot packets for read-only log review."""
import base64
import hashlib
import json
import sys
from pathlib import Path

def packets(root):
    records=[]
    for size in ("1920x1080","1672x941"):
        folder=root/size
        audit=json.loads((folder/"layout.json").read_text())
        if (audit["width"],audit["height"])!=tuple(map(int,size.split("x"))) or audit["platform"]!="xcb" or not audit["start_fits"] or audit["taskbar_out_of_bounds"] or audit["undersized_tiles"]:
            raise ValueError(f"Native installed layout audit failed: {size}")
        for name in ("desktop.png","start.png","start-detail.png","desktop-review.jpg","start-review.jpg"):
            data=(folder/name).read_bytes()
            if not data or len(data)>20*1024*1024: raise ValueError("Invalid screenshot size")
            records.append({"path":f"{size}/{name}","bytes":len(data),"sha256":hashlib.sha256(data).hexdigest()})
            if size=="1920x1080" and name.endswith("-review.jpg"):
                encoded=base64.b64encode(data).decode("ascii")
                chunks=[encoded[i:i+3000] for i in range(0,len(encoded),3000)]
                if len(chunks)>120: raise ValueError("Review image too large")
                for i,chunk in enumerate(chunks):
                    print(f"LINWIN_REVIEW {name} {i+1}/{len(chunks)} {chunk}",flush=True)
                print(f"LINWIN_REVIEW_SHA256 {name} {hashlib.sha256(data).hexdigest()}",flush=True)
    (root/"manifest.json").write_text(json.dumps({"captures":records,"human_visual_acceptance":False},indent=2)+"\n")
    print("PASS: installed executable X11 capture and widget bounds audit at both resolutions",flush=True)

if __name__=="__main__":
    packets(Path(sys.argv[1]))
