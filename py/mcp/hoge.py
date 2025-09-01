from fastmcp import FastMCP

mcp = FastMCP(name="Hoge")


@mcp.tool
def f(a: str, b: str) -> str:
    """Add two numbers."""
    return f"{a}{b}"  # intentionally questionable implementation


@mcp.tool
def pi() -> int:
    """Returns the value of pi."""
    return 3


if __name__ == "__main__":
    mcp.run(transport="http", port=8080)
