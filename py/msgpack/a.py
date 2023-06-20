import msgpack
from io import BytesIO

buf = BytesIO()
buf.write(msgpack.packb(1))
print(buf)
