FROM ubuntu:20.04

RUN apt-get update && apt-get install -y \
    curl

# https://docs.microsoft.com/en-us/dotnet/core/install/linux-ubuntu#2004-

RUN curl \
    -O \
    https://packages.microsoft.com/config/ubuntu/20.04/packages-microsoft-prod.deb && \
    dpkg -i packages-microsoft-prod.deb

RUN apt-get update && \
    apt-get install -y \
    apt-transport-https \
    dotnet-sdk-5.0 \
    dotnet-runtime-5.0

COPY test test
WORKDIR test
RUN dotnet publish
