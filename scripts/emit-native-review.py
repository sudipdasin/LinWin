#!/usr/bin/env python3
"""Small, integrity-checked native screenshot packets for read-only log review."""
import base64
import hashlib
import json
import sys
import struct
from pathlib import Path

def packets(root):
    records=[]
    audits={}
    for size in ("1920x1080","1672x941","xfwm-1920x1080"):
        folder=root/size
        audit=json.loads((folder/"layout.json").read_text())
        if (audit["width"],audit["height"])!=tuple(map(int,size.removeprefix("xfwm-").split("x"))) or audit["platform"]!="xcb" or not audit["start_fits"] or audit["taskbar_out_of_bounds"] or audit["undersized_tiles"] or audit["complete_tiles"]<5 or audit["first_row_tiles"]!=4:
            raise ValueError(f"Native installed layout audit failed: {size}")
        if size.startswith("xfwm-") and not all(audit.get(key) is True for key in ("external_windows_discovered","external_window_activated","external_window_closed","active_window_indicated")):
            raise ValueError("Real XFWM external-window actions failed")
        audits[size]=audit
        for name in ("desktop.png","start.png","start-detail.png","desktop-review.jpg","start-review.jpg","taskbar-review.jpg"):
            data=(folder/name).read_bytes()
            if not data or len(data)>20*1024*1024: raise ValueError("Invalid screenshot size")
            if name.endswith(".png"):
                expected=(audit["start_width"],audit["start_height"]) if name=="start-detail.png" else (audit["width"],audit["height"])
                if not data.startswith(b"\x89PNG\r\n\x1a\n") or struct.unpack(">II",data[16:24])!=expected:
                    raise ValueError(f"Invalid capture dimensions: {size}/{name}")
            elif not (data.startswith(b"\xff\xd8") and data.endswith(b"\xff\xd9")):
                raise ValueError(f"Invalid JPEG: {size}/{name}")
            records.append({"path":f"{size}/{name}","bytes":len(data),"sha256":hashlib.sha256(data).hexdigest()})
            if (size=="1920x1080" and name.endswith("-review.jpg")) or (size.startswith("xfwm-") and name=="taskbar-review.jpg"):
                if size.startswith("xfwm-"): name="xfwm-"+name
                encoded=base64.b64encode(data).decode("ascii")
                chunks=[encoded[i:i+3000] for i in range(0,len(encoded),3000)]
                if len(chunks)>120: raise ValueError("Review image too large")
                for i,chunk in enumerate(chunks):
                    print(f"LINWIN_REVIEW {name} {i+1}/{len(chunks)} {chunk}",flush=True)
                print(f"LINWIN_REVIEW_SHA256 {name} {hashlib.sha256(data).hexdigest()}",flush=True)
    (root/"manifest.json").write_text(json.dumps({"captures":records,"audits":audits,"human_visual_acceptance":False},indent=2)+"\n")
    print("PASS: installed executable X11 capture and widget bounds audit at both resolutions",flush=True)

if __name__=="__main__":
    packets(Path(sys.argv[1]))
