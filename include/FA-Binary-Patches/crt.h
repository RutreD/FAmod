#pragma once

#include "global.h"

typedef int unk_t;
typedef int BOOL;

struct typeInfo
{ // 0x8+ bytes
	void *vtable;
	int zero;
	char *name;
};

template<class S, class T>
struct pair
{
	S first;
	T second;
};

#pragma pack(push, 4)

template <typename T>
struct basic_string
{
	static constexpr uint32_t sso_size = 16 / sizeof(T);
	static constexpr std::size_t maxCapGuard = (1u << 30);
	static constexpr std::size_t npos = static_cast<std::size_t>(-1);

	void* alVal{nullptr}; // allocator cookie / impl detail

	union Bx {
		T* ptr;           // heap pointer when not in SSO
		T  buf[sso_size]; // SSO buffer (15 chars + NUL for char)
	} bx;

	uint32_t mySize{0};           // length (not including NUL)
	uint32_t myRes{sso_size - 1}; // capacity (not including NUL); 15 in SSO

	/**
	 * Default constructor - empty SSO string.
	 */
	basic_string() noexcept : bx() {
		alVal = nullptr;
		bx.buf[0] = T(0);
		mySize = 0;
		myRes = sso_size - 1;
	}

	/**
	 * From C-string: copies via MSVC converting constructor.
	 */
	basic_string(const T* s) {
		if constexpr (std::is_same_v<char, T>)
			reinterpret_cast<void(__thiscall *)(void *, const char *)>(0x405550)(this, s);
		else if constexpr (std::is_same_v<wchar_t, T>)
			reinterpret_cast<void(__thiscall *)(void *, const wchar_t *)>(0x938FD0)(this, s);
	}

	/**
	 * From pointer + length.
	 */
	basic_string(const T* s, std::size_t len) {
		if constexpr (std::is_same_v<char, T>)
			reinterpret_cast<void(__thiscall *)(void *, const char *, std::size_t)>(0x4059E0)(this, s, len);
		else if constexpr (std::is_same_v<wchar_t, T>)
			reinterpret_cast<void(__thiscall *)(void *, const wchar_t *)>(0x938FD0)(this, s);
	}

	/**
	 * From string_view.
	 */
	explicit basic_string(std::basic_string_view<T> sv)
		: basic_string(sv.data(), sv.size()) {}

	/**
	 * From [first, last) pointer range.
	 */
	explicit basic_string(const T* first, const T* last)
		: basic_string(first, (first && last && last >= first) ? static_cast<std::size_t>(last - first) : 0u) {}

	/**
	 * Copy constructor and copy assignment.
	 */
	basic_string(const basic_string& other) {
		if constexpr (std::is_same_v<char, T>)
			reinterpret_cast<void(__thiscall *)(void *, const void *, std::size_t, std::size_t)>(0x4056B0)(this, &other, 0, other.mySize);
		else
			reinterpret_cast<void(__thiscall *)(void *, const void *)>(0x938FD0)(this, other.c_str());
	}

	basic_string& operator=(const basic_string& other) {
		if (this != &other) {
			clear();
			if constexpr (std::is_same_v<char, T>)
				reinterpret_cast<void(__thiscall *)(void *, const void *, std::size_t, std::size_t)>(0x4056B0)(this, &other, 0, other.mySize);
			else
				reinterpret_cast<void(__thiscall *)(void *, const void *)>(0x938FD0)(this, other.c_str());
		}
		return *this;
	}

	/**
	 * Move constructor and move assignment.
	 */
	basic_string(basic_string&& other) noexcept {
		alVal = other.alVal;
		bx = other.bx;
		mySize = other.mySize;
		myRes = other.myRes;
		other.alVal = nullptr;
		other.bx.buf[0] = T(0);
		other.mySize = 0;
		other.myRes = sso_size - 1;
	}

	basic_string& operator=(basic_string&& other) noexcept {
		if (this != &other) {
			clear();
			alVal = other.alVal;
			bx = other.bx;
			mySize = other.mySize;
			myRes = other.myRes;
			other.alVal = nullptr;
			other.bx.buf[0] = T(0);
			other.mySize = 0;
			other.myRes = sso_size - 1;
		}
		return *this;
	}

	/**
	 * Append methods.
	 */
	basic_string& append(const T* s, std::size_t count) {
		if (!s || count == 0) return *this;
		if constexpr (std::is_same_v<char, T>) {
			reinterpret_cast<void*(__thiscall *)(void *, const char *, std::size_t)>(0x4058F0)(this, s, count);
		} else if constexpr (std::is_same_v<wchar_t, T>) {
			reinterpret_cast<void*(__thiscall *)(void *, const void *, std::size_t, std::size_t)>(0x4A1AE0)(this, s, 0, count);
		}
		return *this;
	}

	basic_string& append(const T* s) {
		if (!s) return *this;
		if constexpr (std::is_same_v<char, T>) {
			reinterpret_cast<void*(__thiscall *)(void *, const char *)>(0x405680)(this, s);
		} else if constexpr (std::is_same_v<wchar_t, T>) {
			append(s, std::char_traits<T>::length(s));
		}
		return *this;
	}

	basic_string& append(std::basic_string_view<T> sv) {
		return append(sv.data(), sv.size());
	}

	basic_string& append(const basic_string& other) {
		return append(other.data(), other.size());
	}

	basic_string& operator+=(const basic_string& other) {
		return append(other);
	}

	basic_string& operator+=(std::basic_string_view<T> sv) {
		return append(sv);
	}

	basic_string& operator+=(const T* s) {
		return append(s);
	}

	basic_string& operator+=(T c) {
		return append(&c, 1);
	}

	/**
	 * Pointer access.
	 */
	[[nodiscard]] inline const T* raw_data_unsafe() const noexcept {
		return (myRes < sso_size) ? bx.buf : bx.ptr;
	}

	[[nodiscard]] inline T* raw_data_mut_unsafe() noexcept {
		return (myRes < sso_size) ? bx.buf : bx.ptr;
	}

	[[nodiscard]] inline bool is_sso() const noexcept {
		return myRes < sso_size;
	}

	[[nodiscard]] inline bool basic_sanity() const noexcept {
		if (mySize > maxCapGuard || myRes > maxCapGuard) return false;
		if (mySize > myRes) return false;
		if (!is_sso() && bx.ptr == nullptr) return false;
		return true;
	}

	[[nodiscard]] inline std::size_t size() const noexcept { return mySize; }
	[[nodiscard]] inline std::size_t length() const noexcept { return mySize; }
	[[nodiscard]] inline std::size_t capacity() const noexcept { return myRes; }
	[[nodiscard]] inline bool empty() const noexcept { return mySize == 0; }

	[[nodiscard]] inline const T* data() const noexcept {
		if (!basic_sanity()) {
			if constexpr (std::is_same_v<wchar_t, T>) return L"";
			else return "";
		}
		return raw_data_unsafe();
	}

	[[nodiscard]] inline const T* c_str() const noexcept {
		return data();
	}

	[[nodiscard]] inline std::basic_string_view<T> view() const noexcept {
		return {data(), static_cast<std::size_t>(mySize)};
	}

	constexpr operator std::basic_string_view<T>() const noexcept {
		return view();
	}

	inline void clear() noexcept {
		if (myRes >= sso_size && bx.ptr) {
			reinterpret_cast<void(__cdecl *)(void *)>(0x957A60)(bx.ptr);
		}
		alVal = nullptr;
		bx.buf[0] = T(0);
		mySize = 0;
		myRes = sso_size - 1;
	}

	~basic_string() { clear(); }

	bool operator==(const basic_string& other) const noexcept {
		return view() == other.view();
	}

	bool operator==(std::basic_string_view<T> other) const noexcept {
		return view() == other;
	}

	bool operator==(const T* other) const noexcept {
		return other ? view() == std::basic_string_view<T>(other) : empty();
	}

	auto operator<=>(const basic_string& other) const noexcept {
		return view() <=> other.view();
	}
};

#pragma pack(pop)

using string = basic_string<char>;
using wstring = basic_string<wchar_t>;

VALIDATE_SIZE(string, 0x1C);
VALIDATE_OFFSET(string, bx, 0x04);
VALIDATE_OFFSET(string, mySize, 0x14);
VALIDATE_OFFSET(string, myRes, 0x18);
static_assert(string::sso_size == 0x10);

VALIDATE_SIZE(wstring, 0x1C);
VALIDATE_OFFSET(wstring, bx, 0x04);
VALIDATE_OFFSET(wstring, mySize, 0x14);
VALIDATE_OFFSET(wstring, myRes, 0x18);
static_assert(wstring::sso_size == 0x8);

struct _Container_base
{
	uint32_t gap;
};

template <typename T>
struct vector : _Container_base
{ // 0x10 bytes
	T *begin, *end, *capacity_end;

	T operator[](int i) { return begin[i]; }
	size_t size() { return begin ? end - begin : 0; }
};
VALIDATE_SIZE(vector<unk_t>, 0x10);

template<class T>
struct set : _Container_base
{
	struct _Node
	{
		_Node *_Left;
		_Node *_Parent;
		_Node *_Right;
		T _Myval;
		char _Color;
		char _Isnil;
	};

	_Node *_Myhead;
	size_t _Mysize;
};

template<class K, class V>
struct map : _Container_base {
    struct _Node {
        _Node *_Left;    // 0x00
        _Node *_Parent;  // 0x04
        _Node *_Right;   // 0x08
        struct {
            K first;     // 0x0C
            V second;    // 0x10
        } _Myval;
        char _Color;     // 0x14
        char _Isnil;     // 0x19
    };

    _Node *_Myhead;      // 0x04
    size_t _Mysize;      // 0x08

	size_t size() const {
		return _Mysize;
	}

    struct iterator {
        _Node* _Ptr;
        iterator(_Node* p) : _Ptr(p) {}
        auto* operator->() const { return &(_Ptr->_Myval); }
        bool operator==(const iterator& other) const { return _Ptr == other._Ptr; }
        bool operator!=(const iterator& other) const { return _Ptr != other._Ptr; }
    };
    iterator end() const { 
        return iterator(_Myhead); 
    }

	iterator find(const K& key) const {
        if (!_Myhead || _Mysize == 0) return end();

        _Node* node = _Myhead->_Parent;
        _Node* result = _Myhead;

        while (node && !node->_Isnil) {
            if (key < node->_Myval.first) {
                node = node->_Left;
            } 
            else if (key > node->_Myval.first) {
                node = node->_Right;
            } 
            else {
                result = node;
                break;
            }
        }

        return iterator(result);
    }
	bool contains(const K& key) const {
		return find(key) != end();
	}
};


template <typename T>
struct list : _Container_base
{ // 0xC bytes
	struct _Node
	{
		_Node *_Next;
		_Node *_Prev;
		T _Myval;
	};

	_Node *_Myhead;
	size_t _Mysize;
};
VALIDATE_SIZE(list<unk_t>, 0xC);

#define _DEQUESIZ	(sizeof (_Ty) <= 1 ? 16 \
	: sizeof (_Ty) <= 2 ? 8 \
	: sizeof (_Ty) <= 4 ? 4 \
	: sizeof (_Ty) <= 8 ? 2 : 1)	/* elements per block (a power of 2) */

template<class _Ty>
struct deque : _Container_base
{
	struct block
	{
  		_Ty vals[_DEQUESIZ];
	} * _Map;
	size_t _Mapsize;
	size_t _Myoff;
	size_t _Mysize;
};

template<class T, class _Container = deque<T>>
struct queue
{
	_Container c;
};

namespace detail {

#if defined(_MSC_VER)
class sp_counted_base
{
private:
	sp_counted_base(const sp_counted_base &) = delete;
	sp_counted_base & operator=(const sp_counted_base &) = delete;

public:
	long use_count_{1};
	long weak_count_{1};

	sp_counted_base() noexcept = default;
	virtual ~sp_counted_base() noexcept = default;
	virtual void dispose() noexcept = 0;
	virtual void destroy() noexcept { delete this; }
	virtual void * get_deleter(const void * ti) noexcept { return nullptr; }

	void add_ref_copy() noexcept {
		reinterpret_cast<long(__stdcall *)(volatile long*)>(0xA816B2)(&use_count_);
	}

	void release() noexcept {
		if (reinterpret_cast<long(__stdcall *)(volatile long*)>(0xA816B8)(&use_count_) == 0) {
			dispose();
			weak_release();
		}
	}

	void weak_add_ref() noexcept {
		reinterpret_cast<long(__stdcall *)(volatile long*)>(0xA816B2)(&weak_count_);
	}

	void weak_release() noexcept {
		if (reinterpret_cast<long(__stdcall *)(volatile long*)>(0xA816B8)(&weak_count_) == 0) {
			destroy();
		}
	}

	[[nodiscard]] long use_count() const noexcept {
		return static_cast<long const volatile &>(use_count_);
	}
};

VALIDATE_SIZE(sp_counted_base, 0xC);
VALIDATE_OFFSET(sp_counted_base, use_count_, 0x4);
VALIDATE_OFFSET(sp_counted_base, weak_count_, 0x8);

template<class P>
class sp_counted_impl_p : public sp_counted_base
{
private:
	P ptr;

	sp_counted_impl_p(const sp_counted_impl_p &) = delete;
	sp_counted_impl_p & operator=(const sp_counted_impl_p &) = delete;

public:
	explicit sp_counted_impl_p(P p) noexcept : ptr(p) {}

	void dispose() noexcept override {
		delete[] ptr;
	}
};

template<class P, class D>
class sp_counted_impl_pd : public sp_counted_base
{
private:
	P ptr;
	D del;

	sp_counted_impl_pd(const sp_counted_impl_pd &) = delete;
	sp_counted_impl_pd & operator=(const sp_counted_impl_pd &) = delete;

public:
	sp_counted_impl_pd(P p, D d) noexcept : ptr(p), del(d) {}

	void dispose() noexcept override {
		del(ptr);
	}
};

#else

struct sp_counted_base;

struct sp_counted_base_vftable {
	void* (__thiscall *scalar_deleting_destructor)(sp_counted_base* self, unsigned int flags);
	void  (__thiscall *dispose)(sp_counted_base* self);
	void  (__thiscall *destroy)(sp_counted_base* self);
	void* (__thiscall *get_deleter)(sp_counted_base* self, const void* ti);
};

struct sp_counted_base
{
	const sp_counted_base_vftable *vftable_{nullptr};
	long use_count_{1};
	long weak_count_{1};

	explicit constexpr sp_counted_base(const sp_counted_base_vftable *vt = nullptr) noexcept
		: vftable_(vt), use_count_(1), weak_count_(1) {}
	~sp_counted_base() noexcept = default;

	void add_ref_copy() noexcept {
		reinterpret_cast<long(__stdcall *)(volatile long*)>(0xA816B2)(&use_count_);
	}

	void release() noexcept {
		if (reinterpret_cast<long(__stdcall *)(volatile long*)>(0xA816B8)(&use_count_) == 0) {
			if (vftable_ && vftable_->dispose) vftable_->dispose(this);
			weak_release();
		}
	}

	void weak_add_ref() noexcept {
		reinterpret_cast<long(__stdcall *)(volatile long*)>(0xA816B2)(&weak_count_);
	}

	void weak_release() noexcept {
		if (reinterpret_cast<long(__stdcall *)(volatile long*)>(0xA816B8)(&weak_count_) == 0) {
			if (vftable_ && vftable_->destroy) vftable_->destroy(this);
		}
	}

	[[nodiscard]] long use_count() const noexcept {
		return static_cast<long const volatile &>(use_count_);
	}
};

VALIDATE_SIZE(sp_counted_base, 0xC);
VALIDATE_OFFSET(sp_counted_base, vftable_, 0x0);
VALIDATE_OFFSET(sp_counted_base, use_count_, 0x4);
VALIDATE_OFFSET(sp_counted_base, weak_count_, 0x8);

template<class P>
struct sp_counted_impl_p : public sp_counted_base
{
	P ptr;

	static void* __thiscall DestroyThunk(sp_counted_base* self, unsigned int flags) {
		auto* p = static_cast<sp_counted_impl_p*>(self);
		p->~sp_counted_impl_p();
		if (flags & 1) delete p;
		return self;
	}

	static void __thiscall DisposeThunk(sp_counted_base* self) {
		auto* p = static_cast<sp_counted_impl_p*>(self);
		delete[] p->ptr;
	}

	static void __thiscall DestroyObjThunk(sp_counted_base* self) {
		delete static_cast<sp_counted_impl_p*>(self);
	}

	static void* __thiscall GetDeleterThunk(sp_counted_base*, const void*) {
		return nullptr;
	}

	static inline constexpr sp_counted_base_vftable s_vftable = {
		&DestroyThunk,
		&DisposeThunk,
		&DestroyObjThunk,
		&GetDeleterThunk
	};

	explicit sp_counted_impl_p(P p) noexcept
		: sp_counted_base(&s_vftable), ptr(p) {}
};

template<class P, class D>
struct sp_counted_impl_pd : public sp_counted_base
{
	P ptr;
	D del;

	static void* __thiscall DestroyThunk(sp_counted_base* self, unsigned int flags) {
		auto* p = static_cast<sp_counted_impl_pd*>(self);
		p->~sp_counted_impl_pd();
		if (flags & 1) delete p;
		return self;
	}

	static void __thiscall DisposeThunk(sp_counted_base* self) {
		auto* p = static_cast<sp_counted_impl_pd*>(self);
		p->del(p->ptr);
	}

	static void __thiscall DestroyObjThunk(sp_counted_base* self) {
		delete static_cast<sp_counted_impl_pd*>(self);
	}

	static void* __thiscall GetDeleterThunk(sp_counted_base*, const void*) {
		return nullptr;
	}

	static inline constexpr sp_counted_base_vftable s_vftable = {
		&DestroyThunk,
		&DisposeThunk,
		&DestroyObjThunk,
		&GetDeleterThunk
	};

	sp_counted_impl_pd(P p, D d) noexcept
		: sp_counted_base(&s_vftable), ptr(p), del(d) {}
};

#endif

class shared_count
{
public:
	sp_counted_base * pi_{nullptr};

	constexpr shared_count() noexcept = default;
	constexpr shared_count(sp_counted_base * pi) noexcept : pi_(pi) {}

	template<class P>
	explicit shared_count(P p) : pi_(new sp_counted_impl_p<P>(p)) {}

	template<class P, class D>
	shared_count(P p, D d) : pi_(new sp_counted_impl_pd<P, D>(p, d)) {}

	~shared_count() noexcept {
		if (pi_) pi_->release();
	}

	shared_count(const shared_count & r) noexcept : pi_(r.pi_) {
		if (pi_) pi_->add_ref_copy();
	}

	shared_count(shared_count && r) noexcept : pi_(r.pi_) {
		r.pi_ = nullptr;
	}

	shared_count & operator=(const shared_count & r) noexcept {
		sp_counted_base * tmp = r.pi_;
		if (tmp != pi_) {
			if (tmp) tmp->add_ref_copy();
			if (pi_) pi_->release();
			pi_ = tmp;
		}
		return *this;
	}

	shared_count & operator=(shared_count && r) noexcept {
		if (this != &r) {
			if (pi_) pi_->release();
			pi_ = r.pi_;
			r.pi_ = nullptr;
		}
		return *this;
	}

	void swap(shared_count & r) noexcept {
		sp_counted_base * tmp = r.pi_;
		r.pi_ = pi_;
		pi_ = tmp;
	}

	[[nodiscard]] long use_count() const noexcept {
		return pi_ ? pi_->use_count() : 0;
	}

	[[nodiscard]] bool unique() const noexcept {
		return use_count() == 1;
	}

	[[nodiscard]] bool empty() const noexcept {
		return pi_ == nullptr;
	}
};

} // namespace detail

template<class T>
class shared_ptr
{
public:
	using element_type = T;

	T * px{nullptr};
	detail::shared_count pn{};

	constexpr shared_ptr() noexcept = default;
	constexpr shared_ptr(std::nullptr_t) noexcept : px(nullptr), pn() {}

	template<class Y>
	explicit shared_ptr(Y * p) : px(p), pn(p) {}

	template<class Y, class D>
	shared_ptr(Y * p, D d) : px(p), pn(p, d) {}

	shared_ptr(const shared_ptr & r) noexcept : px(r.px), pn(r.pn) {}

	shared_ptr(shared_ptr && r) noexcept : px(r.px), pn(std::move(r.pn)) {
		r.px = nullptr;
	}

	template<class Y>
	shared_ptr(const shared_ptr<Y> & r) noexcept : px(r.px), pn(r.pn) {}

	template<class Y>
	shared_ptr(shared_ptr<Y> && r) noexcept : px(r.px), pn(std::move(r.pn)) {
		r.px = nullptr;
	}

	shared_ptr & operator=(const shared_ptr & r) noexcept {
		px = r.px;
		pn = r.pn;
		return *this;
	}

	shared_ptr & operator=(shared_ptr && r) noexcept {
		px = r.px;
		pn = std::move(r.pn);
		r.px = nullptr;
		return *this;
	}

	void reset() noexcept {
		px = nullptr;
		pn = detail::shared_count();
	}

	template<class Y>
	void reset(Y * p) {
		shared_ptr(p).swap(*this);
	}

	template<class Y, class D>
	void reset(Y * p, D d) {
		shared_ptr(p, d).swap(*this);
	}

	T & operator*() const noexcept { return *px; }
	T * operator->() const noexcept { return px; }
	T * get() const noexcept { return px; }

	explicit operator bool() const noexcept { return px != nullptr; }

	[[nodiscard]] long use_count() const noexcept { return pn.use_count(); }
	[[nodiscard]] bool unique() const noexcept { return pn.unique(); }

	void swap(shared_ptr & other) noexcept {
		std::swap(px, other.px);
		pn.swap(other.pn);
	}
};
VALIDATE_SIZE(shared_ptr<unk_t>, 0x8);
VALIDATE_OFFSET(shared_ptr<unk_t>, px, 0x0);
VALIDATE_OFFSET(shared_ptr<unk_t>, pn, 0x4);
