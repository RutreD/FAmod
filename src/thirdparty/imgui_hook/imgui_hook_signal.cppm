export module imgui_hook.signal;

import std;

export {
  template <typename Signature> class signal;

  template <typename R, typename... Args> class signal<R(Args...)> {
  public:
    using slot_type = std::function<R(Args...)>;

    signal() = default;
    ~signal() = default;
    signal(const signal &) = delete;
    signal &operator=(const signal &) = delete;
    signal(signal &&) noexcept = default;
    signal &operator=(signal &&) noexcept = default;

    void connect(slot_type slot) { slots_.push_back(std::move(slot)); }

    void operator+=(slot_type slot) { connect(std::move(slot)); }

    void disconnect_all() noexcept { slots_.clear(); }

    [[nodiscard]] std::size_t slot_count() const noexcept {
      return slots_.size();
    }

    [[nodiscard]] bool empty() const noexcept { return slots_.empty(); }

    template <typename... CallArgs>
      requires(sizeof...(CallArgs) == sizeof...(Args))
    void emit(CallArgs &&...args) const {
      for (const auto &slot : slots_) {
        if (slot) {
          slot(std::forward<CallArgs>(args)...);
        }
      }
    }

    class emit_view : public std::ranges::view_interface<emit_view> {
    public:
      emit_view(const signal &sig, std::tuple<Args...> args)
          : slots_(&sig.slots_), args_(std::move(args)) {}

      struct iterator {
        using iterator_concept = std::input_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = R;

        const emit_view *view_{nullptr};
        std::size_t index_{0};

        iterator() = default;
        iterator(const emit_view *view, std::size_t index) noexcept
            : view_(view), index_(index) {}

        R operator*() const {
          return std::apply((*view_->slots_)[index_], view_->args_);
        }

        iterator &operator++() noexcept {
          ++index_;
          return *this;
        }

        void operator++(int) noexcept { ++*this; }

        [[nodiscard]] bool operator==(const iterator &other) const noexcept {
          return index_ == other.index_;
        }
      };

      [[nodiscard]] iterator begin() const noexcept {
        return iterator(this, 0);
      }
      [[nodiscard]] iterator end() const noexcept {
        return iterator(this, slots_ ? slots_->size() : 0);
      }

    private:
      const std::vector<slot_type> *slots_{nullptr};
      std::tuple<Args...> args_;
    };

    template <typename... CallArgs>
      requires(!std::is_void_v<R> && sizeof...(CallArgs) == sizeof...(Args))
    [[nodiscard]] emit_view emit_iterate(CallArgs &&...args) const {
      return emit_view(*this,
                       std::tuple<Args...>(std::forward<CallArgs>(args)...));
    }

  private:
    std::vector<slot_type> slots_;
  };
}