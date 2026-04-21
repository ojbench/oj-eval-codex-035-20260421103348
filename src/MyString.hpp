#ifndef SIMPLE_STRING_SIMPLESTRING_HPP
#define SIMPLE_STRING_SIMPLESTRING_HPP

#include <stdexcept>
#include <cstring>

class MyString {
private:
    union {
        char* heap_ptr;
        char small_buffer[16];
    } storage;
    size_t len_ = 0;          // number of valid characters (excluding nul)
    size_t cap_ = 15;         // capacity excluding nul when using heap; for SSO we also report 15
    bool using_heap_ = false; // true if heap_ptr active

    static size_t sso_capacity() { return 15; }

    bool is_sso() const { return !using_heap_; }

    char* data_ptr() { return using_heap_ ? storage.heap_ptr : storage.small_buffer; }
    const char* data_ptr() const { return using_heap_ ? storage.heap_ptr : storage.small_buffer; }

    void set_nul_at_len() { data_ptr()[len_] = '\0'; }

    void allocate_heap(size_t new_cap) {
        // new_cap excludes nul; allocate new_cap+1 bytes
        char* p = new char[new_cap + 1];
        if (len_ > 0) {
            std::memcpy(p, data_ptr(), len_);
        }
        p[len_] = '\0';
        storage.heap_ptr = p;
        using_heap_ = true;
        cap_ = new_cap;
    }

    void ensure_capacity(size_t needed) {
        // needed excludes nul
        if (is_sso()) {
            if (needed <= sso_capacity()) return;
            // move to heap with growth
            size_t new_cap = sso_capacity();
            while (new_cap < needed) {
                new_cap = new_cap * 2; // simple growth strategy
            }
            size_t old_len = len_;
            char tmp[16];
            if (old_len) std::memcpy(tmp, storage.small_buffer, old_len + 1);
            storage.heap_ptr = new char[new_cap + 1];
            if (old_len) std::memcpy(storage.heap_ptr, tmp, old_len + 1);
            using_heap_ = true;
            cap_ = new_cap;
        } else {
            if (needed <= cap_) return;
            size_t new_cap = cap_ ? cap_ : sso_capacity();
            while (new_cap < needed) new_cap = new_cap * 2;
            char* p = new char[new_cap + 1];
            if (len_) std::memcpy(p, storage.heap_ptr, len_ + 1);
            delete[] storage.heap_ptr;
            storage.heap_ptr = p;
            cap_ = new_cap;
        }
    }

    void shrink_if_needed() {
        if (is_sso()) return; // already compact
        if (len_ <= sso_capacity()) {
            // move back to SSO: save heap pointer before writing into union buffer
            char* old_ptr = storage.heap_ptr;
            if (len_) std::memcpy(storage.small_buffer, old_ptr, len_);
            storage.small_buffer[len_] = '\0';
            delete[] old_ptr;
            using_heap_ = false;
            cap_ = sso_capacity();
            return;
        }
        // optional: shrink heap if much larger than needed
        if (len_ * 2 < cap_) {
            size_t new_cap = len_;
            if (new_cap < sso_capacity()+1) new_cap = len_; // keep on heap since len_>15
            char* p = new char[new_cap + 1];
            if (len_) std::memcpy(p, storage.heap_ptr, len_);
            p[len_] = '\0';
            delete[] storage.heap_ptr;
            storage.heap_ptr = p;
            cap_ = new_cap;
        }
    }

public:
    MyString() {
        using_heap_ = false;
        len_ = 0;
        cap_ = sso_capacity();
        storage.small_buffer[0] = '\0';
    }

    MyString(const char* s) {
        if (!s) {
            using_heap_ = false;
            len_ = 0;
            cap_ = sso_capacity();
            storage.small_buffer[0] = '\0';
            return;
        }
        size_t n = std::strlen(s);
        if (n <= sso_capacity()) {
            using_heap_ = false;
            len_ = n;
            cap_ = sso_capacity();
            std::memcpy(storage.small_buffer, s, n + 1);
        } else {
            using_heap_ = true;
            len_ = n;
            // capacity can be >= n; choose n
            cap_ = n;
            storage.heap_ptr = new char[cap_ + 1];
            std::memcpy(storage.heap_ptr, s, n + 1);
        }
    }

    MyString(const MyString& other) {
        len_ = other.len_;
        using_heap_ = other.using_heap_;
        if (other.is_sso()) {
            cap_ = sso_capacity();
            std::memcpy(storage.small_buffer, other.storage.small_buffer, len_ + 1);
        } else {
            cap_ = other.cap_;
            storage.heap_ptr = new char[cap_ + 1];
            std::memcpy(storage.heap_ptr, other.storage.heap_ptr, len_ + 1);
        }
    }

    MyString(MyString&& other) noexcept {
        len_ = other.len_;
        using_heap_ = other.using_heap_;
        if (other.is_sso()) {
            cap_ = sso_capacity();
            std::memcpy(storage.small_buffer, other.storage.small_buffer, len_ + 1);
        } else {
            cap_ = other.cap_;
            storage.heap_ptr = other.storage.heap_ptr;
            other.storage.heap_ptr = nullptr;
        }
        other.len_ = 0;
        other.using_heap_ = false;
        other.cap_ = sso_capacity();
        other.storage.small_buffer[0] = '\0';
    }

    MyString& operator=(MyString&& other) noexcept {
        if (this == &other) return *this;
        if (using_heap_ && storage.heap_ptr) delete[] storage.heap_ptr;
        len_ = other.len_;
        using_heap_ = other.using_heap_;
        if (other.is_sso()) {
            cap_ = sso_capacity();
            std::memcpy(storage.small_buffer, other.storage.small_buffer, len_ + 1);
        } else {
            cap_ = other.cap_;
            storage.heap_ptr = other.storage.heap_ptr;
            other.storage.heap_ptr = nullptr;
        }
        other.len_ = 0;
        other.using_heap_ = false;
        other.cap_ = sso_capacity();
        other.storage.small_buffer[0] = '\0';
        return *this;
    }

    MyString& operator=(const MyString& other) {
        if (this == &other) return *this;
        if (using_heap_ && storage.heap_ptr) {
            delete[] storage.heap_ptr;
            storage.heap_ptr = nullptr;
        }
        len_ = other.len_;
        using_heap_ = other.using_heap_;
        if (other.is_sso()) {
            cap_ = sso_capacity();
            std::memcpy(storage.small_buffer, other.storage.small_buffer, len_ + 1);
        } else {
            cap_ = other.cap_;
            storage.heap_ptr = new char[cap_ + 1];
            std::memcpy(storage.heap_ptr, other.storage.heap_ptr, len_ + 1);
        }
        return *this;
    }

    ~MyString() {
        if (using_heap_ && storage.heap_ptr) {
            delete[] storage.heap_ptr;
            storage.heap_ptr = nullptr;
        }
    }

    const char* c_str() const { return data_ptr(); }

    size_t size() const { return len_; }

    size_t capacity() const {
        if (is_sso()) return sso_capacity();
        return cap_;
    }

    void reserve(size_t new_capacity) {
        // capacity excluding nul
        if (new_capacity <= capacity()) return;
        ensure_capacity(new_capacity);
        set_nul_at_len();
    }

    void resize(size_t new_size) {
        ensure_capacity(new_size);
        if (new_size > len_) {
            // fill with '\0'
            char* p = data_ptr();
            std::memset(p + len_, 0, new_size - len_);
        }
        len_ = new_size;
        set_nul_at_len();
        shrink_if_needed();
    }

    char& operator[](size_t index) {
        if (index >= len_) throw std::out_of_range("index out of range");
        return data_ptr()[index];
    }

    MyString operator+(const MyString& rhs) const {
        MyString res;
        size_t total = len_ + rhs.len_;
        res.ensure_capacity(total);
        if (len_) std::memcpy(res.data_ptr(), data_ptr(), len_);
        if (rhs.len_) std::memcpy(res.data_ptr() + len_, rhs.data_ptr(), rhs.len_);
        res.len_ = total;
        res.set_nul_at_len();
        return res;
    }

    void append(const char* str) {
        if (!str) return;
        size_t add = std::strlen(str);
        if (add == 0) return;
        ensure_capacity(len_ + add);
        std::memcpy(data_ptr() + len_, str, add);
        len_ += add;
        set_nul_at_len();
    }

    const char& at(size_t pos) const {
        if (pos >= len_) throw std::out_of_range("pos out of range");
        return data_ptr()[pos];
    }

    class const_iterator;

    class iterator {
    private:
        char* ptr_ = nullptr;
    public:
        explicit iterator(char* p = nullptr) : ptr_(p) {}
        iterator& operator++() { ++ptr_; return *this; }
        iterator operator++(int) { iterator tmp(*this); ++(*this); return tmp; }
        iterator& operator--() { --ptr_; return *this; }
        iterator operator--(int) { iterator tmp(*this); --(*this); return tmp; }
        char& operator*() const { return *ptr_; }
        bool operator==(const iterator& other) const { return ptr_ == other.ptr_; }
        bool operator!=(const iterator& other) const { return ptr_ != other.ptr_; }
        bool operator==(const const_iterator& other) const { return (const char*)ptr_ == other.ptr_; }
        bool operator!=(const const_iterator& other) const { return (const char*)ptr_ != other.ptr_; }
        friend class MyString;
    };

    class const_iterator {
    private:
        const char* ptr_ = nullptr;
    public:
        explicit const_iterator(const char* p = nullptr) : ptr_(p) {}
        const_iterator& operator++() { ++ptr_; return *this; }
        const_iterator operator++(int) { const_iterator tmp(*this); ++(*this); return tmp; }
        const_iterator& operator--() { --ptr_; return *this; }
        const_iterator operator--(int) { const_iterator tmp(*this); --(*this); return tmp; }
        const char& operator*() const { return *ptr_; }
        bool operator==(const const_iterator& other) const { return ptr_ == other.ptr_; }
        bool operator!=(const const_iterator& other) const { return ptr_ != other.ptr_; }
        friend class MyString;
    };

public:
    iterator begin() { return iterator(data_ptr()); }
    iterator end() { return iterator(data_ptr() + len_); }
    const_iterator cbegin() const { return const_iterator(data_ptr()); }
    const_iterator cend() const { return const_iterator(data_ptr() + len_); }
};

#endif
