# https://docs.python.org/3/library/http.server.html#module-http.server

# similar to: python3 -m http.server

from http.server import HTTPServer
from http.server import SimpleHTTPRequestHandler


class MyHandler(SimpleHTTPRequestHandler):
    pass


server = HTTPServer(("", 8000), MyHandler)
server.serve_forever()
