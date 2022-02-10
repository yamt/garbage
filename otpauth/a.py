import sys
import time
import base64

from cryptography.hazmat.primitives.twofactor.totp import TOTP
from cryptography.hazmat.primitives.hashes import SHA1

# https://github.com/google/google-authenticator/wiki/Key-Uri-Format

secret = sys.argv[1]  # base32 secret found in otpauth uri
key = base64.b32decode(secret.upper())

# the default values are assumed here
totp = TOTP(key, 6, SHA1(), 30, enforce_key_length=False)
print(totp.generate(time.time()).decode())
