#pragma once
/* Copyright (c) 2016 Kana Shimizu */
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <memory.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <exception>
#include <algorithm>
#include <stddef.h>
#include <string>
#include <vector>
#include <gmp.h>
extern "C"
{
#include "paillier.h"
}

struct Exception:
    std::exception
{
    std::string
	str_;
    virtual ~
    Exception ()
    throw ()
    {
    }
    template <
	class
	T >
	Exception &
    operator<< (const T & param)
    {
	std::ostringstream os;
	os << param;
	str_ += os.str ();
	return *this;
    }
    const char *
    what () const
    throw ()
    {
	return str_.c_str ();
    }
};

inline void
loadData (std::string & data, const std::string & fileName,
	  const char *errMsg)
{
    if (fileName.empty ())
	throw
	Exception () <<
	errMsg <<
	"<empty file name>";
    std::ifstream ifs (fileName.c_str (), std::ios::binary);
    if (ifs)
      {
	  if (ifs >> data)
	    {
		return;
	    }
      }
    throw
    Exception () <<
	errMsg <<
	fileName;
}

inline void
saveData (const std::string & fileName, const std::string & data,
	  const char *errMsg)
{
    if (fileName.empty ())
	throw
	Exception () <<
	errMsg <<
	"<empty file name>";
    std::ofstream ofs (fileName.c_str (), std::ios::binary);
    if (ofs)
      {
	  if (ofs << data)
	    {
		return;
	    }
      }
    throw
    Exception () <<
	errMsg <<
	fileName;
}

class
    PublicKey
{
  public:
    paillier_pubkey_t *
	key_;
    void
    load (const std::string & fileName)
    {
	std::string data;
	loadData (data, fileName, "can't load public key ");
	key_ = paillier_pubkey_from_hex (const_cast < char *>(data.c_str ()));
    }
    void
    save (const std::string & fileName) const
    {
	saveData (fileName, paillier_pubkey_to_hex (key_),
		  "can't save public key ");
    }
    paillier_pubkey_t *
    get ()
    {
	return key_;
    }
    const paillier_pubkey_t *
    get () const
    {
	return
	    key_;
    }
    void
    set (paillier_pubkey_t * key)
    {
	key_ = key;
    }
};

struct PrivateKey
{
    paillier_prvkey_t *
	key_;
    void
    init (PublicKey & publicKey, int keyLen)
    {
	paillier_keygen (keyLen, &publicKey.key_, &key_,
			 paillier_get_rand_devurandom);
    }
    void
    load (const std::string & fileName, const PublicKey & publicKey)
    {
	std::string data;
	loadData (data, fileName, "can't load private key ");
	// QQQ why cast?
	key_ =
	    paillier_prvkey_from_hex (const_cast < char *>(data.c_str ()),
				      const_cast <
				      paillier_pubkey_t *
				      >(publicKey.get ()));
    }
    void
    save (const std::string & fileName) const
    {
	saveData (fileName, paillier_prvkey_to_hex (key_),
		  "can't save private key ");
    }
    paillier_prvkey_t *
    get ()
    {
	return key_;
    }
    const paillier_prvkey_t *
    get () const
    {
	return
	    key_;
    }
    void
    set (paillier_prvkey_t * key)
    {
	key_ = key;
    }
};

inline long long
decrypt (const std::string & resFile, const PublicKey & pub,
	 const PrivateKey & prv)
{
    std::ifstream ifs (resFile.c_str (), std::ios::binary);
    if (!ifs)
      {
	  throw
	  Exception () <<
	      "can't open " <<
	      resFile;
      }
    paillier_plaintext_t
	zero;
    mpz_init (zero.m);
    paillier_plaintext_t *
	two = paillier_plaintext_from_ui (2);
    mpz_div (zero.m, pub.get ()->n, two->m);

    paillier_plaintext_t *
	flag = paillier_plaintext_from_ui (0);
    paillier_ciphertext_t *
	tmp_c = paillier_create_enc_zero ();
    for (;;)
      {
	  std::string line;
	  if (!(ifs >> line))
	      break;
	  int
	      sign = 1;
	  mpz_init_set_str (tmp_c->c, line.c_str (), 16);
	  paillier_dec (flag, const_cast < paillier_pubkey_t * >(pub.get ()),
			const_cast < paillier_prvkey_t * >(prv.get ()),
			tmp_c);
	  // Final output is reported as decimal number
	  if (mpz_cmp (zero.m, flag->m) < 0)
	    {
		mpz_sub (flag->m, pub.get ()->n, flag->m);
		sign = -1;
	    }
	  return (sign * atoll (mpz_get_str (NULL, 10, flag->m)));
      }
}


inline void
encrypt (const std::string & fileName, const PublicKey & pub,
	 long long in_val)
{
    std::ofstream ofs (fileName.c_str (), std::ios::binary);
    if (!ofs)
      throw
	Exception () <<
	"can't open " <<
	fileName;

    paillier_plaintext_t *
	val;

    if (in_val < 0)
      {
	in_val *= -1;
	val = paillier_plaintext_from_ui (0);
	mpz_sub_ui (val->m, pub.get ()->n, in_val);
      }
    else
      {
	val = paillier_plaintext_from_ui (in_val);
      }
    ofs << mpz_get_str (0, 16,
			paillier_enc (NULL,
				      const_cast <
				      paillier_pubkey_t * >(pub.get ()),
				      val,
				      paillier_get_rand_devurandom)->c)
	<< std::endl;
}

// overload function: encrypt long long array
inline void
encrypt (const std::string & fileName, const PublicKey & pub,
	 long long* in_val, int valDimension)
{
    std::ofstream ofs (fileName.c_str (), std::ios::binary);
    if (!ofs)
      throw
	Exception () <<
	"can't open " <<
	fileName;

    paillier_plaintext_t *
	val;

    for (int i=0; i<valDimension; i++) 
      {
	if (in_val[i] < 0)
	  {
	    in_val[i] *= -1;
	    val = paillier_plaintext_from_ui (0);
	    mpz_sub_ui (val->m, pub.get ()->n, in_val[i]);
	  }
	else
	  {
	    val = paillier_plaintext_from_ui (in_val[i]);
	  }
	ofs << mpz_get_str (0, 16,
			    paillier_enc (NULL,
					  const_cast <
					  paillier_pubkey_t * >(pub.get ()),
					  val,
					  paillier_get_rand_devurandom)->c)
	    << std::endl;
      }
}

inline void
addall (std::vector < std::string > fileNames, const std::string & oFileName,
	const PublicKey & pub, int dimension)
{
    std::vector < paillier_ciphertext_t * > tmp_c;
    std::vector < paillier_ciphertext_t * > all;
    paillier_pubkey_t *
	pkey = const_cast < paillier_pubkey_t * >(pub.get ());

    for (int i = 0; i < dimension; i++) {
      tmp_c.push_back (paillier_create_enc_zero ());
      all.push_back (paillier_create_enc_zero ());
    }
    

    for (int i = 0; i < (int) fileNames.size (); i++)
      {
	  std::ifstream ifs (fileNames[i].c_str (), std::ios::binary);
	  if (!ifs)
	      throw
	      Exception () <<
	      "can't open " <<
	      fileNames[i];
	  std::string line;
	  for (int j = 0; j < dimension; j++)
	    {
	      if (!(ifs >> line))
		break;
	      mpz_init_set_str (tmp_c[j]->c, line.c_str (), 16);
	      paillier_mul (pkey, all[j], all[j], tmp_c[j]);
	    }
      }

    std::ofstream ofs (oFileName.c_str (), std::ios::binary);
    if (!ofs)
	throw
	Exception () <<
	"can't open " <<
	oFileName;

    for (int i = 0; i < dimension; i++) 
      ofs << mpz_get_str (0, 16, all[i]->c) << std::endl;

}
