#!/usr/bin/env python3
"""
Word frequency analyzer.
Usage:
    python3 wordcloud.py <text>
    python3 wordcloud.py --file <filepath>
"""
import sys
import json
import re
import os
from collections import Counter

# Common stopwords to skip
STOPWORDS = {
    'the','a','an','and','or','but','in','on','at','to','for','of','with',
    'is','are','was','were','be','been','being','have','has','had','do','does',
    'did','will','would','could','should','may','might','shall','can','need',
    'i','you','he','she','it','we','they','me','him','her','us','them',
    'my','your','his','its','our','their','this','that','these','those',
    'what','which','who','whom','how','when','where','why','all','each',
    'every','both','few','more','most','other','some','such','no','not',
    'only','same','so','than','too','very','just','from','as','by','about',
    'into','through','during','before','after','above','below','between',
    'out','up','down','if','then','because','while','although','though',
    'however','therefore','also','already','still','yet','again','here',
    'there','now','get','got','go','going','come','came','take','made',
    'make','know','think','see','look','want','use','find','give','tell',
    'work','call','try','ask','need','feel','become','leave','put','mean',
    'keep','let','begin','show','hear','play','run','move','live','believe',
    'hold','bring','happen','write','provide','sit','stand','lose','pay',
    'meet','include','continue','set','learn','change','lead','understand',
    'watch','follow','stop','create','speak','read','spend','grow','open',
    'walk','win','offer','remember','love','consider','appear','buy','wait',
    'serve','die','send','expect','build','stay','fall','cut','reach','kill',
    'remain','suggest','raise','pass','sell','require','report','decide',
    'pull','am','re','ve','ll','s','t','d','m','o','p','q','u','v','w',
    'x','y','z','a','b','c','e','f','g','h','j','k','l','n','r',
}

def analyze(text: str) -> dict:
    # Clean text
    words = re.findall(r"[a-zA-Z']+", text.lower())
    words = [w.strip("'") for w in words if len(w) > 2 and w not in STOPWORDS]

    if not words:
        return {'error': 'No meaningful words found in the text'}

    total_words = len(re.findall(r'\S+', text))
    total_chars = len(text)
    sentences = len(re.findall(r'[.!?]+', text)) or 1
    paragraphs = len([p for p in text.split('\n\n') if p.strip()]) or 1
    unique_words = len(set(words))
    avg_word_len = sum(len(w) for w in words) / len(words) if words else 0

    # Reading time (avg 200 words/min)
    reading_secs = int((total_words / 200) * 60)
    if reading_secs < 60:
        reading_time = f"{reading_secs} seconds"
    else:
        reading_time = f"{reading_secs // 60}m {reading_secs % 60}s"

    # Top words
    counter = Counter(words)
    top20 = counter.most_common(20)

    # Most used letters
    letters = re.findall(r'[a-z]', text.lower())
    top_letters = Counter(letters).most_common(5)

    # Lexical diversity (unique/total)
    diversity = round(unique_words / len(words) * 100, 1) if words else 0

    return {
        'total_words': total_words,
        'unique_words': unique_words,
        'total_chars': total_chars,
        'sentences': sentences,
        'paragraphs': paragraphs,
        'avg_word_len': round(avg_word_len, 1),
        'reading_time': reading_time,
        'lexical_diversity': diversity,
        'top_words': [{'word': w, 'count': c} for w, c in top20],
        'top_letters': [{'letter': l, 'count': c} for l, c in top_letters],
    }

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print(json.dumps({'error': 'No input provided'}))
        sys.exit(1)

    if sys.argv[1] == '--file':
        if len(sys.argv) < 3:
            print(json.dumps({'error': 'No file path provided'}))
            sys.exit(1)
        filepath = sys.argv[2]
        if not os.path.exists(filepath):
            print(json.dumps({'error': f'File not found: {filepath}'}))
            sys.exit(1)
        try:
            with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
                text = f.read()
        except Exception as e:
            print(json.dumps({'error': str(e)}))
            sys.exit(1)
    else:
        text = ' '.join(sys.argv[1:])

    result = analyze(text)
    print(json.dumps(result))
