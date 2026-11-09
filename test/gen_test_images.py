#!/usr/bin/env python3
"""Generate test document images for LLAH-Takeda functional testing."""

import numpy as np
import cv2
import os

OUT_DB = os.path.join(os.path.dirname(__file__), "db_images")
OUT_QUERY = os.path.join(os.path.dirname(__file__), "query_images")
os.makedirs(OUT_DB, exist_ok=True)
os.makedirs(OUT_QUERY, exist_ok=True)

W, H = 640, 480


def draw_text_block(img, texts, x0, y0, font_scale=0.5, thickness=1, line_gap=22):
    for i, text in enumerate(texts):
        cv2.putText(img, text, (x0, y0 + i * line_gap), cv2.FONT_HERSHEY_SIMPLEX, font_scale, 0, thickness, cv2.LINE_AA)


# --- DB images (5 different "documents") ---
docs = [
    [
        "Document A - Page 1",
        "The quick brown fox jumps",
        "over the lazy dog.",
        "ABCDEFGHIJKLMNOP",
        "1234567890 !@#$%",
        "Lorem ipsum dolor sit amet",
        "consectetur adipiscing elit",
        "Sed do eiusmod tempor",
        "incididunt ut labore",
        "et dolore magna aliqua",
        "Ut enim ad minim veniam",
        "quis nostrud exercitation",
        "ullamco laboris nisi ut",
        "aliquip ex ea commodo",
        "consequat",
    ],
    [
        "Document B - Page 2",
        "Pack my box with five",
        "dozen liquor jugs.",
        "QRSTUVWXYZ abcdef",
        "ghijklmnopqrstuv wxyz",
        "Duis aute irure dolor in",
        "reprehenderit in voluptate",
        "velit esse cillum dolore",
        "eu fugiat nulla pariatur",
        "Excepteur sint occaecat",
        "cupidatat non proident",
        "sunt in culpa qui officia",
        "deserunt mollit anim id",
        "est laborum dolor sit",
        "amet consectetur",
    ],
    [
        "Document C - Page 3",
        "How vexingly quick daft",
        "zebras jump!",
        "0123456789 ABCDEF",
        "The five boxing wizards",
        "jump quickly at dawn",
        "Sphinx of black quartz",
        "judge my vow today",
        "Jackdaws love my big",
        "sphinx of quartz now",
        "The jay pig fox zebra",
        "and my wolves quack",
        "Brick quiz whangs jumpy",
        "veldt fox and the cow",
        "Many dogs jumped",
    ],
    [
        "Document D - Page 4",
        "Computer Science 101:",
        "Data structures & Algorithms",
        "Binary Search Tree O(logn)",
        "Hash Table O(1) average",
        "Graph BFS DFS traversal",
        "Dynamic Programming method",
        "Greedy Algorithm approach",
        "Divide and Conquer tech",
        "Sorting: Quick Merge Heap",
        "Machine Learning basics",
        "Neural Network layers",
        "Optimization gradient",
        "Parallel computing GPU",
        "Cache memory hierarchy",
    ],
    [
        "Document E - Page 5",
        "Mathematics Reference:",
        "Calculus: dy/dx = lim h->0",
        "Linear Algebra: Ax = b",
        "Probability P(A|B) Bayes",
        "Statistics mean std dev",
        "Geometry: pi*r^2 area",
        "Trigonometry sin cos tan",
        "Number Theory: primes",
        "Set Theory: union inter",
        "Graph Theory: V E edges",
        "Topology: open closed",
        "Combinatorics: nCr nPr",
        "Differential equations",
        "Fourier Transform DFT",
    ],
]

for i, lines in enumerate(docs):
    img = np.ones((H, W), dtype=np.uint8) * 255
    # NOTE: no closed page border. The retrieval-side noise removal fills contour
    # interiors, so a closed 1px frame would blank out the whole page.
    cv2.line(img, (20, 55), (W - 20, 55), 0, 1)
    draw_text_block(img, lines[:1], 30, 45, font_scale=0.6, thickness=2)
    draw_text_block(img, lines[1:], 30, 78, font_scale=0.45, thickness=1, line_gap=26)
    fname = os.path.join(OUT_DB, f"doc{i + 1:03d}.jpg")
    cv2.imwrite(fname, img)
    print(f"Created: {fname}")

# --- Query images: slightly modified versions of doc1 and doc3 ---
# Query 1: doc1 with minor noise (should match doc1)
img_q1 = cv2.imread(os.path.join(OUT_DB, "doc001.jpg"), cv2.IMREAD_GRAYSCALE)
noise = np.random.normal(0, 3, img_q1.shape).astype(np.int16)
img_q1 = np.clip(img_q1.astype(np.int16) + noise, 0, 255).astype(np.uint8)
q1path = os.path.join(OUT_QUERY, "query_doc001.jpg")
cv2.imwrite(q1path, img_q1)
print(f"Created: {q1path}")

# Query 2: doc3 cropped slightly (should match doc3)
img_q2 = cv2.imread(os.path.join(OUT_DB, "doc003.jpg"), cv2.IMREAD_GRAYSCALE)
img_q2 = img_q2[10 : H - 10, 10 : W - 10]
img_q2 = cv2.resize(img_q2, (W, H))
q2path = os.path.join(OUT_QUERY, "query_doc003.jpg")
cv2.imwrite(q2path, img_q2)
print(f"Created: {q2path}")

# Query 3: exact copy of doc5 (should definitely match)
img_q3 = cv2.imread(os.path.join(OUT_DB, "doc005.jpg"), cv2.IMREAD_GRAYSCALE)
q3path = os.path.join(OUT_QUERY, "query_doc005.jpg")
cv2.imwrite(q3path, img_q3)
print(f"Created: {q3path}")

print("\nDone. DB images: 5, Query images: 3")
