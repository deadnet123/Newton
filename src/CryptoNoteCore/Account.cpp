// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
//
// This file is part of Bytecoin.
//
// Bytecoin is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Bytecoin is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Bytecoin.  If not, see <http://www.gnu.org/licenses/>.

#include "Account.h"
#include "CryptoNoteSerialization.h"
#include "crypto/keccak.c"

namespace CryptoNote {
//-----------------------------------------------------------------
AccountBase::AccountBase() {
  setNull();
}
//-----------------------------------------------------------------
void AccountBase::setNull() {
  m_keys = AccountKeys();
}
//-----------------------------------------------------------------
void AccountBase::generateViewFromSpend(Crypto::SecretKey &spend, Crypto::SecretKey &viewSecret, Crypto::PublicKey &viewPublic) {
  Crypto::SecretKey viewKeySeed;
  keccak((uint8_t *)&spend, sizeof(spend), (uint8_t *)&viewKeySeed, sizeof(viewKeySeed));
  Crypto::generate_keys_from_seed(viewPublic, viewSecret, viewKeySeed);
}
//-----------------------------------------------------------------
void AccountBase::generate() {
  Crypto::generate_keys(m_keys.address.spendPublicKey, m_keys.spendSecretKey);
/* We derive the view secret key by taking our spend secret key, hashing
   with keccak-256, and then using this as the seed to generate a new set
   of keys - the public and private view keys. See generate_keys_from_seed */
generateViewFromSpend(m_keys.spendSecretKey, m_keys.viewSecretKey, m_keys.address.viewPublicKey);
  m_creation_timestamp = time(NULL);
}
//-----------------------------------------------------------------
void AccountBase::generateDeterministic() { 
  Crypto::SecretKey second;
  Crypto::generate_keys(m_keys.address.spendPublicKey, m_keys.spendSecretKey);
  keccak((uint8_t *)&m_keys.spendSecretKey, sizeof(Crypto::SecretKey), (uint8_t *)&second, sizeof(Crypto::SecretKey));
  Crypto::generate_keys_from_seed(m_keys.address.viewPublicKey, m_keys.viewSecretKey, second);
  m_creation_timestamp = time(NULL);
}
//-----------------------------------------------------------------
Crypto::SecretKey AccountBase::generate_key(const Crypto::SecretKey& recovery_key, bool recover, bool two_random)
{
  Crypto::SecretKey first = generate_m_keys(m_keys.address.spendPublicKey, m_keys.spendSecretKey, recovery_key, recover);

  // rng for generating second set of keys is hash of first rng.  means only one set of electrum-style words needed for recovery
  Crypto::SecretKey second;
  keccak((uint8_t *)&first, sizeof(Crypto::SecretKey), (uint8_t *)&second, sizeof(Crypto::SecretKey));

  generate_m_keys(m_keys.address.viewPublicKey, m_keys.viewSecretKey, second, two_random ? false : true);

  struct tm timestamp;
  timestamp.tm_year = 2018 - 1900;  // year 2018
  timestamp.tm_mon = 9 - 1;  // month Aug
  timestamp.tm_mday = 30;  // 30 of Aug
  timestamp.tm_hour = 0;
  timestamp.tm_min = 0;
  timestamp.tm_sec = 0;

  if (recover)
  {
    m_creation_timestamp = mktime(&timestamp);
  }
    else
  {
    m_creation_timestamp = time(NULL);
  }
  return first;
}
//-----------------------------------------------------------------
const AccountKeys &AccountBase::getAccountKeys() const {
  return m_keys;
}

void AccountBase::setAccountKeys(const AccountKeys &keys) {
  m_keys = keys;
}
//-----------------------------------------------------------------

void AccountBase::serialize(ISerializer &s) {
  s(m_keys, "m_keys");
  s(m_creation_timestamp, "m_creation_timestamp");
}
}
