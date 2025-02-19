/*
 * Z2k.h
 *
 */

#ifndef MATH_Z2K_H_
#define MATH_Z2K_H_

#include <mpirxx.h>
#include <string>
using namespace std;

#include "Tools/avx_memcpy.h"
#include "bigint.h"
#include "field_types.h"
#include "mpn_fixed.h"
#include "ValueInterface.h"

template <int K>
class Z2 : public ValueInterface
{
protected:
	template <int L>
	friend class Z2;
	friend class bigint;

	static const int N_WORDS = ((K + 7) / 8 + sizeof(mp_limb_t) - 1)
			/ sizeof(mp_limb_t);
	static const int N_LIMB_BITS = 8 * sizeof(mp_limb_t);
	static const uint64_t UPPER_MASK = uint64_t(-1LL) >> (N_LIMB_BITS - 1 - (K - 1) % N_LIMB_BITS);

	mp_limb_t a[N_WORDS];


public:
	typedef void Inp;
	typedef void PO;
	typedef void Square;

	class DummyZ2kProtocol
	{
	public:
	    static void assign(Z2& _,  const Z2& __, int& ___)
        {
            (void) _, (void) __, (void) ___;
            throw not_implemented();
        }
	};
	typedef DummyZ2kProtocol Protocol;

	static const int N_BITS = K;
	static const int N_BYTES = (K + 7) / 8;

	static int size() { return N_BYTES; }
	static int size_in_limbs() { return N_WORDS; }
	static int t() { return 0; }

	static char type_char() { return 'R'; }
	static string type_string() { return "Z2^" + to_string(int(N_BITS)); }

	static DataFieldType field_type() { return DATA_INT; }

	static const bool invertible = false;

	template <int L, int M>
	static Z2<K> Mul(const Z2<L>& x, const Z2<M>& y);

	static void reqbl(int n);
	static bool allows(Dtype dtype);

	typedef Z2 next;
	typedef Z2 Scalar;

	Z2() { assign_zero(); }
	Z2(uint64_t x) : Z2() { a[0] = x; }
	Z2(__m128i x) : Z2() { avx_memcpy(a, &x, min(N_BYTES, 16)); }
	Z2(int x) : Z2(long(x)) { a[N_WORDS - 1] &= UPPER_MASK; }
	Z2(long x) : Z2(uint64_t(x)) { if (K > 64 and x < 0) memset(&a[1], -1, N_BYTES - 8); }
	Z2(const Integer& x);
	Z2(const bigint& x);
	Z2(const void* buffer) : Z2() { assign(buffer); }
	template <int L>
	Z2(const Z2<L>& x) : Z2()
	{ avx_memcpy(a, x.a, min(N_BYTES, x.N_BYTES)); normalize(); }

	void normalize() { a[N_WORDS - 1] &= UPPER_MASK; }

	void assign_zero() { avx_memzero(a, sizeof(a)); }
	void assign_one()  { assign_zero(); a[0] = 1; }
	void assign(const void* buffer) { avx_memcpy(a, buffer, N_BYTES); normalize(); }
	void assign(int x) { *this = x; }

	mp_limb_t get_limb(int i) const { return a[i]; }
	bool get_bit(int i) const;

	const void* get_ptr() const { return a; }
	const mp_limb_t* get() const { return a; }

	void convert_destroy(bigint& a) { *this = a; }

	void negate() { 
		throw not_implemented();
	}
	
	Z2<K> operator+(const Z2<K>& other) const;
	Z2<K> operator-(const Z2<K>& other) const;

	template <int L>
	Z2<K+L> operator*(const Z2<L>& other) const;

	Z2<K> operator*(bool other) const { return other ? *this : Z2<K>(); }

	Z2<K> operator/(const Z2& other) const { (void) other; throw not_implemented(); }

	Z2<K>& operator+=(const Z2<K>& other);
	Z2<K>& operator-=(const Z2<K>& other);

	Z2<K> operator<<(int i) const;
	Z2<K> operator>>(int i) const;

	bool operator==(const Z2<K>& other) const;
	bool operator!=(const Z2<K>& other) const { return not (*this == other); }

	void add(const Z2<K>& a, const Z2<K>& b) { *this = a + b; }
	void add(const Z2<K>& a) { *this += a; }
	void sub(const Z2<K>& a, const Z2<K>& b) { *this = a - b; }

	template <int M, int L>
	void mul(const Z2<M>& a, const Z2<L>& b) { *this = Z2<K>::Mul(a, b); }
	template <int L>
	void mul(const Integer& a, const Z2<L>& b) { *this = Z2<K>::Mul(Z2<64>(a), b); }

	void mul(const Z2& a) { *this = Z2::Mul(*this, a); }

	template <int t>
	void add(octetStream& os) { add(os.consume(size())); }

	Z2& invert();
	void invert(const Z2& a) { *this = a; invert(); }

	Z2 sqrRoot();

	bool is_zero() const { return *this == Z2<K>(); }
	bool is_one() const { return *this == 1; }
	bool is_bit() const { return is_zero() or is_one(); }

	void SHL(const Z2& a, const bigint& i) { *this = a << i.get_ui(); }
	void SHR(const Z2& a, const bigint& i) { *this = a >> i.get_ui(); }

	void AND(const Z2& a, const Z2& b);
	void OR(const Z2& a, const Z2& b);
	void XOR(const Z2& a, const Z2& b);

	void randomize(PRNG& G, int n = -1);
	void almost_randomize(PRNG& G) { randomize(G); }

	void force_to_bit() { throw runtime_error("impossible"); }

	void pack(octetStream& o, int = -1) const;
	void unpack(octetStream& o, int n = -1);

	void input(istream& s, bool human=true);
	void output(ostream& s, bool human=true) const;

	template <int J>
	friend ostream& operator<<(ostream& o, const Z2<J>& x);
};

template<int K>
class SignedZ2 : public Z2<K>
{
public:
    SignedZ2()
    {
    }

    template <int L>
    SignedZ2(const SignedZ2<L>& other) : Z2<K>(other)
    {
        if (K < L and other.negative())
        {
            this->a[Z2<K>::N_WORDS - 1] |= ~Z2<K>::UPPER_MASK;
            for (int i = Z2<K>::N_WORDS; i < this->N_WORDS; i++)
                this->a[i] = -1;
        }
    }

    SignedZ2(const Integer& other) : SignedZ2(SignedZ2<64>(other))
    {
    }

    template<class T>
    SignedZ2(const T& other) :
            Z2<K>(other)
    {
    }

    bool negative() const
    {
        return this->a[this->N_WORDS - 1] & 1ll << ((K - 1) % (8 * sizeof(mp_limb_t)));
    }

    SignedZ2 operator-() const
    {
        return SignedZ2() - *this;
    }

    SignedZ2 operator-(const SignedZ2& other) const
    {
        return Z2<K>::operator-(other);
    }

    void output(ostream& s, bool human = true) const;
};

template<int K>
inline Z2<K> Z2<K>::operator+(const Z2<K>& other) const
{
    Z2<K> res;
    mpn_add_fixed_n<N_WORDS>(res.a, a, other.a);
    res.a[N_WORDS - 1] &= UPPER_MASK;
    return res;
}

template<int K>
Z2<K> Z2<K>::operator-(const Z2<K>& other) const
{
	Z2<K> res;
	mpn_sub_fixed_n<N_WORDS>(res.a, a, other.a);
	res.a[N_WORDS - 1] &= UPPER_MASK;
	return res;
}

template <int K>
inline Z2<K>& Z2<K>::operator+=(const Z2<K>& other)
{
	mpn_add_fixed_n<N_WORDS>(a, other.a, a);
	a[N_WORDS - 1] &= UPPER_MASK;
	return *this;
}

template <int K>
Z2<K>& Z2<K>::operator-=(const Z2<K>& other)
{
	*this = *this - other;
	return *this;
}

template <int K>
template <int L, int M>
inline Z2<K> Z2<K>::Mul(const Z2<L>& x, const Z2<M>& y)
{
	Z2<K> res;
	mpn_mul_fixed_<N_WORDS, Z2<L>::N_WORDS, Z2<M>::N_WORDS>(res.a, x.a, y.a);
	res.a[N_WORDS - 1] &= UPPER_MASK;
	return res;
}

template <int K>
template <int L>
inline Z2<K+L> Z2<K>::operator*(const Z2<L>& other) const
{
	return Z2<K+L>::Mul(*this, other);
}

template <int K>
Z2<K> Z2<K>::operator<<(int i) const
{
	Z2<K> res;
	int n_limb_shift = i / N_LIMB_BITS;
	for (int j = n_limb_shift; j < N_WORDS; j++)
		res.a[j] = a[j - n_limb_shift];
	int n_inside_shift = i % N_LIMB_BITS;
	if (n_inside_shift > 0)
		mpn_lshift(res.a, res.a, N_WORDS, n_inside_shift);
	res.a[N_WORDS - 1] &= UPPER_MASK;
	return res;
}

template <int K>
Z2<K> Z2<K>::operator>>(int i) const
{
	Z2<K> res;
	int n_byte_shift = i / 8;
	memcpy(res.a, (char*)a + n_byte_shift, N_BYTES - n_byte_shift);
	int n_inside_shift = i % 8;
	if (n_inside_shift > 0)
		mpn_rshift(res.a, res.a, N_WORDS, n_inside_shift);
	return res;
}

template<int K>
void Z2<K>::randomize(PRNG& G, int n)
{
	(void) n;
	G.get_octets<N_BYTES>((octet*)a);
	normalize();
}

template<int K>
void Z2<K>::pack(octetStream& o, int n) const
{
	(void) n;
	o.append((octet*)a, N_BYTES);
}

template<int K>
void Z2<K>::unpack(octetStream& o, int n)
{
	(void) n;
	o.consume((octet*)a, N_BYTES);
}

template<int K>
void to_gfp(Z2<K>& res, const bigint& a)
{
	res = a;
}

template<int K>
SignedZ2<K> abs(const SignedZ2<K>& x)
{
    if (x.negative())
        return -x;
    else
        return x;
}

template<int K>
void SignedZ2<K>::output(ostream& s, bool human) const
{
    if (human)
    {
        bigint::tmp = *this;
        s << bigint::tmp;
    }
    else
        Z2<K>::output(s, false);
}

template<int K>
ostream& operator<<(ostream& o, const SignedZ2<K>& x)
{
    x.output(o, true);
    return o;
}

template<int K>
inline void to_signed_bigint(bigint& res, const SignedZ2<K>& x, int n)
{
    bigint tmp = x;
    to_signed_bigint(res, tmp, n);
}

#endif /* MATH_Z2K_H_ */
