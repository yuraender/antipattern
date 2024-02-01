#pragma once

#include <vector>

template<typename T, typename U>
class Iterator {
  public:
  Iterator() = default;

  explicit Iterator(const T& data)
      : data(data) {}

  virtual Iterator<T, U>& operator++() = 0;

  virtual U operator*() const = 0;

  protected:
  T data;
  size_t current = 0;
};

template<typename T>
class VectorIterator : public Iterator<std::vector<T>, T> {
  public:
  VectorIterator() = default;

  explicit VectorIterator(const std::vector<T>& data)
      : Iterator<std::vector<T>, T>(data) {}

  VectorIterator<T>& operator++() override {
    this->current++;
    return *this;
  }

  T operator*() const override {
    if (this->current < 0 || this->current > this->data.size() - 1) {
      return nullptr;
    }
    return this->data[this->current];
  }
};
