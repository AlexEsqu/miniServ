#!/usr/bin/env python3
import sys
import os

# Read POST data from stdin
post_data = sys.stdin.read()
content_type = os.environ.get('CONTENT_TYPE', '')

print("Status: 200 OK")
print("Content-Type: text/html")
print()  # Empty line required

print("<html><body>")
print(f"<h1>Received {len(post_data)} bytes</h1>")
print(f"<p>Content-Type: {content_type}</p>")
print(f"<pre>{post_data}</pre>")
print("</body></html>")
