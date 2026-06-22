#generate the RSA private key
openssl genpkey -outform PEM -algorithm RSA -pkeyopt rsa_keygen_bits:2048 -out priv.key

#Convert the private key to PKCS#1 and DER
#THIS IS THE CRUCIAL STEP FOR THE KEY ON AN ESP8266
openssl rsa -inform PEM -outform DER -traditional -passin pass: -in priv.key -out key.der

#Create the CSR (see csrconfig.txt)
openssl req -new -nodes -key priv.key -config csrconfig.txt -nameopt utf8 -utf8 -out cert.csr

#Self-sign your CSR (see certconfig.txt)
openssl req -x509 -nodes -in cert.csr -days 1095 -key priv.key -config certconfig.txt -extensions req_ext -nameopt utf8 -utf8 -out cert.crt

#convert cert to DER
openssl x509 -in cert.crt -out cert.der -outform DER