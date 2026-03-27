#!/usr/bin/env python3
"""
URL tools: encode, decode, extract links from text.
Usage:
    python3 urltool.py encode <text>
    python3 urltool.py decode <url>
    python3 urltool.py extract <text>
    python3 urltool.py extract --file <filepath>
"""
import sys
import json
import re
import os
from urllib.parse import quote, unquote, urlparse, parse_qs

def encode_url(text: str) -> dict:
    encoded = quote(text, safe='')
    encoded_safe = quote(text, safe=':/?#[]@!$&\'()*+,;=-._~')
    return {
        'original': text,
        'fully_encoded': encoded,
        'safe_encoded': encoded_safe,
    }

def decode_url(url: str) -> dict:
    decoded = unquote(url)
    # Also try to parse query params
    try:
        parsed = urlparse(decoded)
        params = parse_qs(parsed.query)
        params_clean = {k: v[0] if len(v) == 1 else v for k, v in params.items()}
    except Exception:
        parsed = None
        params_clean = {}

    result = {
        'original': url,
        'decoded': decoded,
    }
    if parsed and parsed.scheme:
        result['scheme'] = parsed.scheme
        result['host'] = parsed.netloc
        result['path'] = parsed.path
        if params_clean:
            result['query_params'] = params_clean
        if parsed.fragment:
            result['fragment'] = parsed.fragment
    return result

def extract_links(text: str) -> dict:
    # Match URLs including those without http
    url_pattern = re.compile(
        r'(?:https?://|ftp://|www\.)'
        r'(?:[^\s<>"{}|\\^`\[\]]*)'
        r'(?:[^\s<>"{}|\\^`\[\].,;:!?\'"\)])'
        r'|'
        r'(?:[a-zA-Z0-9](?:[a-zA-Z0-9-]{0,61}[a-zA-Z0-9])?\.)'
        r'(?:com|org|net|edu|gov|io|co|uk|us|de|fr|jp|cn|in|pk|ae|sa|bd|ng|br|ru|info|biz|app|dev|tech|online)'
        r'(?:/[^\s<>"{}|\\^`\[\]]*)?',
        re.IGNORECASE
    )

    matches = url_pattern.findall(text)
    # Deduplicate preserving order
    seen = set()
    unique_urls = []
    for url in matches:
        url = url.strip('.,;:!?\'")')
        if url and url not in seen:
            seen.add(url)
            unique_urls.append(url)

    # Categorize
    social = []
    media = []
    docs = []
    other = []

    social_domains = ['facebook', 'twitter', 'instagram', 'youtube', 'tiktok',
                      'linkedin', 'snapchat', 'pinterest', 'reddit', 'whatsapp',
                      'telegram', 'discord', 'twitch']
    media_ext = ['.jpg', '.jpeg', '.png', '.gif', '.mp4', '.mp3', '.pdf',
                 '.webp', '.svg', '.avi', '.mov', '.wav']
    doc_domains = ['docs.google', 'drive.google', 'dropbox', 'onedrive',
                   'notion', 'github', 'gitlab', 'pastebin']

    for url in unique_urls:
        url_lower = url.lower()
        if any(s in url_lower for s in social_domains):
            social.append(url)
        elif any(url_lower.endswith(e) or e in url_lower for e in media_ext):
            media.append(url)
        elif any(d in url_lower for d in doc_domains):
            docs.append(url)
        else:
            other.append(url)

    return {
        'total': len(unique_urls),
        'all_urls': unique_urls,
        'social': social,
        'media': media,
        'documents': docs,
        'other': other,
    }

if __name__ == '__main__':
    if len(sys.argv) < 3:
        print(json.dumps({'error': 'Usage: urltool.py <encode|decode|extract> <text>'}))
        sys.exit(1)

    mode = sys.argv[1].lower()

    if mode == 'extract' and sys.argv[2] == '--file':
        if len(sys.argv) < 4:
            print(json.dumps({'error': 'No file path'}))
            sys.exit(1)
        filepath = sys.argv[3]
        if not os.path.exists(filepath):
            print(json.dumps({'error': f'File not found: {filepath}'}))
            sys.exit(1)
        with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
            text = f.read()
        print(json.dumps(extract_links(text)))
    else:
        text = ' '.join(sys.argv[2:])
        if mode == 'encode':
            print(json.dumps(encode_url(text)))
        elif mode == 'decode':
            print(json.dumps(decode_url(text)))
        elif mode == 'extract':
            print(json.dumps(extract_links(text)))
        else:
            print(json.dumps({'error': f'Unknown mode: {mode}'}))
            sys.exit(1)
