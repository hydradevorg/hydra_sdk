/*
* OctetString
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_SYMKEY_H_
#define BOTAN_SYMKEY_H_

#include <botan/secmem.h>
#include <string>
#include <span>

namespace Botan {

class RandomNumberGenerator;

/**
* Octet String
*/
class BOTAN_PUBLIC_API(2,0) OctetString final
   {
   public:
      /**
      * @return size of this octet string in bytes
      */
      size_t length() const { return m_data.size(); }
      size_t size() const { return m_data.size(); }
      bool empty() const { return m_data.empty(); }

      /**
      * @return this object as a secure_vector<uint8_t>
      */
      secure_vector<uint8_t> bits_of() const { return m_data; }

      /**
      * @return start of this string
      */
      const uint8_t* begin() const { return m_data.data(); }

      /**
      * @return end of this string
      */
      const uint8_t* end() const   { return begin() + m_data.size(); }

      /**
      * @return this encoded as hex
      */
      std::string to_string() const;

      /**
      * XOR the contents of another octet string into this one
      * @param other octet string
      * @return reference to this
      */
      OctetString& operator^=(const OctetString& other);

      /**
      * Force to have odd parity
      *
      * Deprecated. There is no reason to use this outside of interacting with
      * some very old or weird system which requires DES and also which do not
      * automatically ignore the parity bits.
      */
      BOTAN_DEPRECATED("Why would you need to do this")
      void set_odd_parity();

      /**
      * Create a new OctetString
      * @param str is a hex encoded string
      */
      explicit OctetString(std::string_view str = "");

      /**
      * Create a new random OctetString
      * @param rng is a random number generator
      * @param len is the desired length in bytes
      */
      OctetString(RandomNumberGenerator& rng, size_t len);

      /**
      * Create a new OctetString
      * @param in is an array
      * @param len is the length of in in bytes
      */
      OctetString(const uint8_t in[], size_t len);

      /**
      * Create a new OctetString
      * @param in a bytestring
      */
      explicit OctetString(std::span<const uint8_t> in) : m_data(in.begin(), in.end()) {}

      /**
      * Create a new OctetString
      * @param in a bytestring
      */
      explicit OctetString(secure_vector<uint8_t> in) : m_data(std::move(in)) {}

   private:
      secure_vector<uint8_t> m_data;
   };

/**
* Compare two strings
* @param x an octet string
* @param y an octet string
* @return if x is equal to y
*/
BOTAN_PUBLIC_API(2,0) bool operator==(const OctetString& x,
                          const OctetString& y);

/**
* Compare two strings
* @param x an octet string
* @param y an octet string
* @return if x is not equal to y
*/
BOTAN_PUBLIC_API(2,0) bool operator!=(const OctetString& x,
                          const OctetString& y);

/**
* Concatenate two strings
* @param x an octet string
* @param y an octet string
* @return x concatenated with y
*/
BOTAN_PUBLIC_API(2,0) OctetString operator+(const OctetString& x,
                                const OctetString& y);

/**
* XOR two strings
* @param x an octet string
* @param y an octet string
* @return x XORed with y
*/
BOTAN_PUBLIC_API(2,0) OctetString operator^(const OctetString& x,
                                const OctetString& y);


/**
* Alternate name for octet string showing intent to use as a key
*/
using SymmetricKey = OctetString;

/**
* Alternate name for octet string showing intent to use as an IV
*/
using InitializationVector = OctetString;

}

#endif
