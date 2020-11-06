#include "util/encryption.h"
#include "cryptopp/aes.h"
#include "cryptopp/base64.h"
#include "cryptopp/eccrypto.h"
#include "cryptopp/filters.h"
#include "cryptopp/modes.h"
#include "cryptopp/oids.h"
#include "cryptopp/osrng.h"
#include "util/config.h"
#include <iomanip>
#include <iostream>

namespace broccoli {

void Encryption::GenerateEccKeys(std::string &prv_key, std::string &pub_key) {

  CryptoPP::AutoSeededRandomPool rnd(false);

  CryptoPP::ECIES<CryptoPP::ECP>::PrivateKey privateKey;
  CryptoPP::ECIES<CryptoPP::ECP>::PublicKey publicKey;

  privateKey.Initialize(rnd, CryptoPP::ASN1::secp128r1());

  privateKey.MakePublicKey(publicKey);

  CryptoPP::ECIES<CryptoPP::ECP>::Encryptor encryptor(publicKey);
  CryptoPP::Base64Encoder pub_encoder(new CryptoPP::StringSink(pub_key), false);
  publicKey.DEREncode(pub_encoder);
  pub_encoder.MessageEnd();

  CryptoPP::ECIES<CryptoPP::ECP>::Decryptor decryptor(privateKey);
  CryptoPP::Base64Encoder prv_encoder(new CryptoPP::StringSink(prv_key), false);
  privateKey.DEREncode(prv_encoder);
  prv_encoder.MessageEnd();
}

std::string Encryption::EccEncrypt(const std::string &pub_key, const std::string &plain_text) {
  assert(pub_key.size() == 76);
  std::string cipher_text;

  CryptoPP::StringSource pub_string(pub_key, true, new CryptoPP::Base64Decoder);
  CryptoPP::ECIES<CryptoPP::ECP>::Encryptor encryptor(pub_string);

  size_t cipher_text_size = encryptor.CiphertextLength(plain_text.size());

  cipher_text.resize(cipher_text_size);
  CryptoPP::RandomPool rnd;
  encryptor.Encrypt(rnd, (CryptoPP::byte *)(plain_text.c_str()), plain_text.size(),
                    (CryptoPP::byte *)(cipher_text.data()));
  return cipher_text;
}

std::string Encryption::EccDecrypt(const std::string &prv_key, const std::string &cipher_text) {
  assert(prv_key.size() == 64);
  std::string plain_text;

  CryptoPP::StringSource privString(prv_key, true, new CryptoPP::Base64Decoder);
  CryptoPP::ECIES<CryptoPP::ECP>::Decryptor decryptor(privString);
  auto plain_text_size = decryptor.MaxPlaintextLength(cipher_text.size());
  plain_text.resize(plain_text_size);
  CryptoPP::RandomPool rnd;
  decryptor.Decrypt(rnd, (CryptoPP::byte *)cipher_text.c_str(), cipher_text.size(),
                    (CryptoPP::byte *)plain_text.data());
  return plain_text;
}

void Encryption::GenerateAesKey(std::string &key) {
  CryptoPP::byte byte_key[CryptoPP::AES::DEFAULT_KEYLENGTH];
  CryptoPP::AutoSeededRandomPool rnd(false);
  rnd.GenerateBlock(byte_key, CryptoPP::AES::DEFAULT_KEYLENGTH);

  CryptoPP::Base64Encoder key_encoder(new CryptoPP::StringSink(key), false);
  key_encoder.Put(byte_key, CryptoPP::AES::DEFAULT_KEYLENGTH);
  key_encoder.MessageEnd();
}

std::string Encryption::AesEncrypt(const std::string &key, const std::string &plain_text) {
  assert(key.size() == 24);

  CryptoPP::byte iv[CryptoPP::AES::BLOCKSIZE];
  CryptoPP::AutoSeededRandomPool rnd(false);
  rnd.GenerateBlock(iv, CryptoPP::AES::BLOCKSIZE);
  std::string string_iv;
  string_iv.assign(reinterpret_cast<const char *>(iv), CryptoPP::AES::BLOCKSIZE);

  CryptoPP::StringSource string_key(key, true, new CryptoPP::Base64Decoder);
  CryptoPP::byte byte_key[CryptoPP::AES::DEFAULT_KEYLENGTH];
  string_key.Get(byte_key, CryptoPP::AES::DEFAULT_KEYLENGTH);
  CryptoPP::AES::Encryption aes_encryption(byte_key, CryptoPP::AES::DEFAULT_KEYLENGTH);

  CryptoPP::CBC_Mode_ExternalCipher::Encryption cbc_encryption(aes_encryption, iv);

  std::string cipher_text;
  CryptoPP::StreamTransformationFilter stf_encryptor(cbc_encryption, new CryptoPP::StringSink(cipher_text));
  stf_encryptor.Put(reinterpret_cast<const unsigned char *>(plain_text.c_str()), plain_text.length());
  stf_encryptor.MessageEnd();

  return string_iv + cipher_text;
}

std::string Encryption::AesDecrypt(const std::string &key, const std::string &cipher_text) {
  assert(key.size() == 24);

  CryptoPP::byte byte_key[CryptoPP::AES::DEFAULT_KEYLENGTH];
  CryptoPP::StringSource string_key(key, true, new CryptoPP::Base64Decoder);
  string_key.Get(byte_key, CryptoPP::AES::DEFAULT_KEYLENGTH);
  CryptoPP::AES::Decryption aes_encryption(byte_key, CryptoPP::AES::DEFAULT_KEYLENGTH);

  std::string string_iv;
  string_iv.assign(cipher_text.c_str(), CryptoPP::AES::BLOCKSIZE);

  CryptoPP::byte iv[CryptoPP::AES::BLOCKSIZE];
  memcpy(iv, string_iv.c_str(), CryptoPP::AES::BLOCKSIZE);
  CryptoPP::CBC_Mode_ExternalCipher::Decryption cbc_decryption(aes_encryption, iv);

  std::string string_cipher_text;
  string_cipher_text.assign(cipher_text.c_str() + CryptoPP::AES::BLOCKSIZE,
                            cipher_text.size() - CryptoPP::AES::BLOCKSIZE);

  std::string plant_text;
  CryptoPP::StreamTransformationFilter stf_decryptor(cbc_decryption, new CryptoPP::StringSink(plant_text));
  stf_decryptor.Put(reinterpret_cast<const unsigned char *>(string_cipher_text.c_str()), string_cipher_text.size());
  stf_decryptor.MessageEnd();

  return plant_text;
}

void Encryption::PrintStringInHex(const std::string s) {
  for (auto c : s) {
    std::cout << std::hex << std::setw(2) << std::setiosflags(std::ios::uppercase);
    std::cout << std::setfill('0') << (0xFF & c) << ' ';
  }
  std::cout << std::endl;
}

} // namespace broccoli
