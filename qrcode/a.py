# https://github.com/lincolnloop/python-qrcode
# pip3.9 install --user qrcode
import qrcode

qr = qrcode.QRCode(
    version=1,
    error_correction=qrcode.constants.ERROR_CORRECT_H,
    box_size=10,
    border=4,
)
qr.add_data("hoge")

# is this necessary?
# qr.make(fit=True)

qr.print_ascii()
