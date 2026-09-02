import argparse
import time
import urllib.request
import json

url = "http://localhost:8000/v1/chat/completions"
model = None
streaming = True
prompt = "Hi, please suggest a topic to chat."
dump = False


def ts():
    return time.perf_counter()


def query(messages):
    data = {
        "messages": messages,
        "stream": streaming,
    }
    if model is not None:
        data["model"] = model
    headers = {
        "Content-Type": "application/json",
        "Accept": "application/json",
    }
    data = json.dumps(data).encode("utf-8")
    if dump:
        print(f"REQUEST: url={url}, data={data}, headers={headers}")
    req = urllib.request.Request(url=url, data=data, headers=headers)
    start = ts()
    with urllib.request.urlopen(req) as resp:
        if dump:
            for k, v in resp.headers.items():
                 print(f"RESPONSE HEADER: {k}: {v}")
        if streaming:
            msg = do_stream(resp, start)
        else:
            msg = do_non_stream(resp, start)
    return msg


def do_non_stream(resp, start):
    resp = resp.read().decode()
    j = json.loads(resp)
    u = j.get('usage')
    try:
        msg = j["choices"][0]["message"]["content"]
    except KeyError:
        print(f"unexpect response {j}")
        exit(1)
    # print(f"dump json response {j}")
    print(f"{msg}")
    if u is not None and dump:
        print(f"USAGE {u}")
    return msg


# note: some implementations returns empty "choices" immediately
# before "[DONE]". probably to report usage statistics?
#
# {
#   "id": "chatcmpl-c4d0bd3d-b4f7-50b8-a15c-e5f5f25400fd",
#   "model": "test-model",
#   "object": "chat.completion.chunk",
#   "created": 1786591160,
#   "usage": {
#     "prompt_tokens": 14,
#     "completion_tokens": 64,
#     "total_tokens": 78,
#     "prompt_tokens_details": {
#       "cached_tokens": 0
#     }
#   },
#   "choices": []
# }

def do_stream(resp, start):
    msg = ""
    first = None
    ntokens = 0
    got_empty = False
    u = None
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
            break
        assert not got_empty
        j = json.loads(line)
        d = None
        try:
            if len(j["choices"]) == 0:
                got_empty = True
            else:
                d = j["choices"][0]["delta"]
        except (KeyError, IndexError):
            print(f"unexpect response {j}")
            exit(1)
        if d is not None:
            token = d.get("content")
            if token:
                print(token, end="", flush=True)
                msg += token
        nu = j.get('usage')
        if nu is not None and u is not None:
            print("Get multiple usage. is this possible?")
            exit(1) # XXX
        u = nu
    print("")
    if u is not None and dump:
        print(f"USAGE {u}")
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


parser = argparse.ArgumentParser()
parser.add_argument("--model")
parser.add_argument("--url")
parser.add_argument("--streaming", action='store_true')
parser.add_argument("--prompt")
parser.add_argument("--dump", action='store_true')
args = parser.parse_args()
if args.url is not None:
    url = args.url
model = args.model
if args.streaming is not None:
    streaming = args.streaming
if args.prompt is not None:
    prompt = args.prompt
if args.dump is not None:
    dump = args.dump


messages = []
# messages.append(
#    {
#        "role": "system",
#        "content": "",
#    }
# )
messages.append({"role": "user", "content": prompt})
sep = "=" * 16
count = 1
print(f"{messages[-1]['content']}")
while True:
    print(f"[{count}] {sep}")
    count += 1
    # print(f"context: {json.dumps(messages, indent=4)}")
    try:
        resp = query(messages)
    except urllib.error.HTTPError as e:
        resp = e.fp.read()
        print(f"HTTPError code={e.code} body={resp}")
        for k, v in e.headers.items():
            print(f"RESPONSE HEADER {k} = {v}")
        if e.code != 400:
            raise
        olen = len(messages)
        if olen <= 8:
            raise
        messages = forget(messages)
        nlen = len(messages)
        print(f"forgot {olen - nlen} messages out of {olen} messages")
        continue
    messages.append({"role": "assistant", "content": resp})
    if len(messages) > 1 and messages[-1]["content"] == messages[-2]["content"]:
        print("REPEATING IDENTICAL MESSAGES!")
    flip_roles(messages)
