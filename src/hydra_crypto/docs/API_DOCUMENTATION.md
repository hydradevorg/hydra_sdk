# Hydra Edge KMS API Documentation

## Overview

The Hydra Edge KMS (Key Management System) provides a secure local API server for generating and managing cryptographic keys using post-quantum algorithms. The API server runs on port 8888 and is accessible only to authorized local parties.

## Authentication

All API endpoints (except `/health`) require authentication using an API key. The API key must be provided in the `Authorization` header as a Bearer token:

```
Authorization: Bearer <your_api_key>
```

The API key is generated when the server starts and is displayed in the console output.

## Base URL

```
http://localhost:8888
```

## API Endpoints

### Health Check

**Endpoint:** `GET /health`

**Description:** Check if the API server is running.

**Authentication:** Not required

**Response:**
```json
{
  "status": "ok",
  "timestamp": 1746317794
}
```

### Root Key Management

#### Generate Root Key Pair

**Endpoint:** `POST /api/root/generate`

**Description:** Generate a new root key pair for signing and encrypting public keys.

**Authentication:** Required

**Request Body:**
```json
{
  "strength": "ML-DSA-65" // Optional, can be ML-DSA-44, ML-DSA-65, or ML-DSA-87
}
```

**Response:**
```json
{
  "id": "unique_key_id",
  "dilithium_public_key": "base64_encoded_dilithium_public_key",
  "kyber_public_key": "base64_encoded_kyber_public_key",
  "strength": "ML-DSA-65",
  "created_at": 1746317794,
  "expires_at": 1754093794,
  "is_active": true
}
```

#### Get Active Root Key

**Endpoint:** `GET /api/root/active`

**Description:** Get the currently active root key pair.

**Authentication:** Required

**Response:**
```json
{
  "id": "unique_key_id",
  "dilithium_public_key": "base64_encoded_dilithium_public_key",
  "kyber_public_key": "base64_encoded_kyber_public_key",
  "strength": "ML-DSA-65",
  "created_at": 1746317794,
  "expires_at": 1754093794,
  "is_active": true
}
```

#### List Root Keys

**Endpoint:** `GET /api/root/list`

**Description:** List all root key pairs.

**Authentication:** Required

**Response:**
```json
[
  {
    "id": "unique_key_id_1",
    "dilithium_public_key": "base64_encoded_dilithium_public_key",
    "kyber_public_key": "base64_encoded_kyber_public_key",
    "strength": "ML-DSA-65",
    "created_at": 1746317794,
    "expires_at": 1754093794,
    "is_active": true
  },
  {
    "id": "unique_key_id_2",
    "dilithium_public_key": "base64_encoded_dilithium_public_key",
    "kyber_public_key": "base64_encoded_kyber_public_key",
    "strength": "ML-DSA-65",
    "created_at": 1746317794,
    "expires_at": 1754093794,
    "is_active": false
  }
]
```

#### Rotate Root Key

**Endpoint:** `POST /api/root/rotate`

**Description:** Rotate the active root key pair. This will generate a new root key pair and mark it as active.

**Authentication:** Required

**Request Body:**
```json
{
  "force": false // Optional, if true will force rotation even if the current key hasn't expired
}
```

**Response:**
```json
{
  "id": "new_unique_key_id",
  "dilithium_public_key": "base64_encoded_dilithium_public_key",
  "kyber_public_key": "base64_encoded_kyber_public_key",
  "strength": "ML-DSA-65",
  "created_at": 1746317794,
  "expires_at": 1754093794,
  "is_active": true
}
```

#### Sign Public Key

**Endpoint:** `POST /api/root/sign`

**Description:** Sign a public key using the active root key.

**Authentication:** Required

**Request Body:**
```json
{
  "public_key": "base64_encoded_public_key"
}
```

**Response:**
```json
{
  "public_key": "base64_encoded_public_key",
  "signature": "base64_encoded_signature",
  "root_key_id": "unique_key_id"
}
```

#### Verify Signed Public Key

**Endpoint:** `POST /api/root/verify`

**Description:** Verify a signed public key.

**Authentication:** Required

**Request Body:**
```json
{
  "public_key": "base64_encoded_public_key",
  "signature": "base64_encoded_signature",
  "root_key_id": "unique_key_id" // Optional, if not provided the active key will be used
}
```

**Response:**
```json
{
  "valid": true // or false if the signature is invalid
}
```

#### Encrypt Public Key

**Endpoint:** `POST /api/root/encrypt`

**Description:** Encrypt a public key using the active root key.

**Authentication:** Required

**Request Body:**
```json
{
  "public_key": "base64_encoded_public_key"
}
```

**Response:**
```json
{
  "encrypted_public_key": "base64_encoded_encrypted_public_key",
  "root_key_id": "unique_key_id"
}
```

#### Decrypt Public Key

**Endpoint:** `POST /api/root/decrypt`

**Description:** Decrypt an encrypted public key.

**Authentication:** Required

**Request Body:**
```json
{
  "encrypted_public_key": "base64_encoded_encrypted_public_key",
  "root_key_id": "unique_key_id" // Optional, if not provided the ID embedded in the encrypted key will be used
}
```

**Response:**
```json
{
  "public_key": "base64_encoded_public_key"
}
```

### Kyber-AES Hybrid Encryption

#### Generate Key Pair

**Endpoint:** `POST /api/kyber-aes/generate`

**Description:** Generate a new Kyber key pair for hybrid encryption with AES.

**Authentication:** Required

**Request Body:** Empty JSON object `{}`

**Response:**
```json
{
  "public_key": "base64_encoded_public_key",
  "private_key": "base64_encoded_private_key"
}
```

#### Encrypt Data

**Endpoint:** `POST /api/kyber-aes/encrypt`

**Description:** Encrypt data using Kyber-AES hybrid encryption.

**Authentication:** Required

**Request Body:**
```json
{
  "public_key": "base64_encoded_public_key",
  "plaintext": "data_to_encrypt"
}
```

**Response:**
```json
{
  "ciphertext": "base64_encoded_ciphertext"
}
```

#### Decrypt Data

**Endpoint:** `POST /api/kyber-aes/decrypt`

**Description:** Decrypt data using Kyber-AES hybrid encryption.

**Authentication:** Required

**Request Body:**
```json
{
  "private_key": "base64_encoded_private_key",
  "ciphertext": "base64_encoded_ciphertext"
}
```

**Response:**
```json
{
  "plaintext": "decrypted_data"
}
```

### Kyber KEM (Key Encapsulation Mechanism)

#### Generate Key Pair

**Endpoint:** `POST /api/kyber/generate`

**Description:** Generate a new Kyber KEM key pair (ML-KEM-768).

**Authentication:** Required

**Request Body:** Empty JSON object `{}`

**Response:**
```json
{
  "public_key": "base64_encoded_public_key",
  "private_key": "base64_encoded_private_key"
}
```

#### Encapsulate Shared Secret

**Endpoint:** `POST /api/kyber/encapsulate`

**Description:** Encapsulate a shared secret using a public key.

**Authentication:** Required

**Request Body:**
```json
{
  "public_key": "base64_encoded_public_key"
}
```

**Response:**
```json
{
  "ciphertext": "base64_encoded_ciphertext",
  "shared_secret": "base64_encoded_shared_secret"
}
```

#### Decapsulate Shared Secret

**Endpoint:** `POST /api/kyber/decapsulate`

**Description:** Decapsulate a shared secret using a private key and ciphertext.

**Authentication:** Required

**Request Body:**
```json
{
  "private_key": "base64_encoded_private_key",
  "ciphertext": "base64_encoded_ciphertext"
}
```

**Response:**
```json
{
  "shared_secret": "base64_encoded_shared_secret"
}
```

### Dilithium Signature

#### Generate Key Pair

**Endpoint:** `POST /api/dilithium/generate`

**Description:** Generate a new Dilithium signature key pair (ML-DSA).

**Authentication:** Required

**Request Body:**
```json
{
  "strength": "ML-DSA-65" // Optional, can be ML-DSA-44, ML-DSA-65, or ML-DSA-87
}
```

**Response:**
```json
{
  "public_key": "base64_encoded_public_key",
  "private_key": "base64_encoded_private_key",
  "strength": "ML-DSA-65"
}
```

#### Sign Message

**Endpoint:** `POST /api/dilithium/sign`

**Description:** Sign a message using a private key.

**Authentication:** Required

**Request Body:**
```json
{
  "private_key": "base64_encoded_private_key",
  "message": "message_to_sign",
  "strength": "ML-DSA-65" // Must match the strength used for key generation
}
```

**Response:**
```json
{
  "signature": "base64_encoded_signature"
}
```

#### Verify Signature

**Endpoint:** `POST /api/dilithium/verify`

**Description:** Verify a signature using a public key.

**Authentication:** Required

**Request Body:**
```json
{
  "public_key": "base64_encoded_public_key",
  "message": "original_message",
  "signature": "base64_encoded_signature",
  "strength": "ML-DSA-65" // Must match the strength used for key generation
}
```

**Response:**
```json
{
  "valid": true // or false if the signature is invalid
}
```

## Error Handling

All API endpoints return appropriate HTTP status codes and error messages in JSON format:

```json
{
  "error": "Error message describing what went wrong"
}
```

Common status codes:
- 200: Success
- 400: Bad Request (missing or invalid parameters)
- 401: Unauthorized (invalid or missing API key)
- 500: Internal Server Error

## Example Usage

See the `scripts/debug_api.sh` script for examples of how to use the API endpoints.

## Security Considerations

- The API server is configured to only accept connections from localhost.
- All requests must include a valid API key in the Authorization header.
- Private keys should be stored securely and never shared.
- The API server does not persist keys; they are only held in memory during the session.
- Root keys are used to sign and encrypt public keys to ensure their authenticity and confidentiality.
- Key rotation is implemented to periodically update keys and maintain security.
