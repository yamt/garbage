import time
import urllib.request
import json

url = "http://localhost:8000/v1/chat/completions"


def ts():
    return time.perf_counter()


def query(messages):
    data = {
        "messages": messages,
        "stream": True,
    }
    headers = {
        "Content-Type": "application/json",
        "Accept": "application/json",
    }
    msg = ""
    data = json.dumps(data).encode("utf-8")
    start = ts()
    first = None
    ntokens = 0
    req = urllib.request.Request(url=url, data=data, headers=headers)
    with urllib.request.urlopen(req) as resp:
        for line in resp:
            line = line.decode()
            # print(line)
            if not line.startswith("data: "):
                continue
            line = line[6:].strip()
            ntokens += 1
            if first is None:
                first = ts()
            if line == "[DONE]":
                print("")
                break
            line = json.loads(line)
            d = line["choices"][0]["delta"]
            token = d.get("content")
            if token:
                print(token, end="", flush=True)
                msg += token
    if ntokens > 1:
        ttft = first - start
        tps = (ntokens - 1) / (ts() - first)
        print(f"Got {ntokens} tokens, TTFT {ttft} TPS {tps}")
    return msg


def flip_roles(messages):
    d = {
        "user": "assistant",
        "assistant": "user",
    }
    for m in messages:
        r = m["role"]
        if r in d:
            m["role"] = d[r]


def forget(messages):
    sys = [m for m in messages if m["role"] == "system"]
    keep = (len(messages) - len(sys)) // 2
    return sys + messages[-keep:]


messages = []
messages.append(
    {
        "role": "system",
        "content": "Suggest the next topic proactively instead of saying good bye.",
    }
)
messages.append({"role": "user", "content": "Hi, please suggest a topic to chat."})
print(f"{messages[-1]["content"]}")
while True:
    print("=" * 16)
    # print(f"context: {json.dumps(messages, indent=4)}")
    try:
        resp = query(messages)
    except urllib.error.HTTPError as e:
        if e.code != 400:
            raise
        resp = e.fp.read()
        print(f"HTTPError code={e.code} body={resp}")
        olen = len(messages)
        messages = forget(messages)
        nlen = len(messages)
        print(f"forgot {olen - nlen} messages out of {olen} messages")
        continue
    messages.append({"role": "assistant", "content": resp})
    flip_roles(messages)
