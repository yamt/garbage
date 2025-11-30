# a quick-and-dirty script to render the topology from a hedgehog config.
# cf. https://docs.hedgehog.cloud/latest/install-upgrade/build-wiring/
# only a small subset of fields is implemented.
#
# eg.
# < hh.yml python hhviz.py|dot -Granksep=2 -Tpng -o out.png

import yaml
import sys

vpccolor = "red"
swcolor = "blue"

objs = {}
connections = {}

j = yaml.safe_load_all(sys.stdin)


def replace(n):
    return n.replace("/", "_").replace("-", "_")


def add_obj(obj, **kwargs):
    if obj not in objs:
        objs[obj] = {"ports": []}
    objs[obj] |= kwargs


def add_port(p):
    obj, port = p.split("/", 1)
    if obj not in objs:
        add_obj(obj)
    objs[obj]["ports"].append(port)


def add_vpc_attachment(vpc, conn):
    if vpc not in objs:
        add_obj(vpc)
    objs[vpc]["ports"].append(conn)


def add_connection(name, p1, p2, attachment):
    if name not in connections:
        connections[name] = {"attachments": [], "links": []}
    if p1 is not None:
        connections[name]["links"].append((p1, p2))
    if attachment is not None:
        connections[name]["attachments"].append(attachment)


for e in j:
    kind = e["kind"]
    spec = e["spec"]
    metadata = e["metadata"]
    name = metadata["name"]
    if kind == "Server":
        add_obj(name, kind=kind)
    elif kind == "Switch":
        add_obj(name, kind=kind)
    elif kind == "VPC":
        add_obj(name, kind=kind, spec=spec)
    elif kind == "Connection":
        type = list(spec)[0]
        if type not in {"unbundled", "bundled"}:
            print(f"unknown Connection {type}", file=sys.stderr)
        links = spec[type].get("links")
        if links is None:
            links = [spec[type]["link"]]
        for link in links:
            if "server" in link:
                p1 = link["server"]
                p2 = link["switch"]
            else:
                p1, p2 = link.values()
            server_port = p1["port"]
            sw_port = p2["port"]
            add_port(server_port)
            add_port(sw_port)
            add_connection(name, server_port, sw_port, None)
    elif kind == "VPCAttachment":
        conn = spec["connection"]
        # subnet = spec["subnet"]
        add_connection(conn, None, None, spec)
        # vpc, subnet = subnet.split("/", 1)
        # add_vpc_attachment(vpc, conn)
    else:
        print(f"unknown kind {kind}", file=sys.stderr)

print(f"graph net {{")
for name, e in objs.items():
    kind = e["kind"]
    print(f"subgraph cluster_{kind}_{replace(name)} {{")
    print(f'label = "{name}"')
    if kind == "VPC":
        print(f'color = "{vpccolor}"')
        for k, v in e["spec"]["subnets"].items():
            label = f"{k}\\n{v['subnet']}\\nvlan {v['vlan']}"
            subnetnode = f"subnet_{name}_{k}"
            print(f'{subnetnode} [label="{label}"]')
    if kind == "Switch":
        print(f'color = "{swcolor}"')
    for p in e["ports"]:
        pname = replace(f"{name}/{p}")
        if kind == "VPC":
            label = ""
            shape = "circle"
        else:
            label = p
            shape = "box"
        print(f'{pname} [label="{label}" shape={shape}]')
    print(f"}}")
for name, e in connections.items():
    for l in e["links"]:
        server_port, sw_port = l
        print(f"{replace(server_port)} -- {replace(sw_port)}")
        attachments = e.get("attachments")
        if attachments:
            for a in attachments:
                vpc, subnet = a['subnet'].split("/", 1)
                vpc_conn = f"{vpc}/{name}"
                subnetnode = f"subnet_{vpc}_{subnet}"
                if a.get('nativeVLAN', False):
                    style = "solid" # untagged vlan
                else:
                    style = "dotted" # tagged vlan
                print(f"{replace(sw_port)} -- {subnetnode} [style={style}]")
print(f"}}")
