#pragma once

#include <vector>

template<typename T, typename U>
class Iterator {
  public:
  Iterator() {}

  Iterator(const T& data)
      : data(data) {}

  virtual Iterator<T, U>& operator++() = 0;

  virtual U operator*() const = 0;

  private:
  T data{};
  size_t current = 0;
};

template<typename T>
class VectorIterator : Iterator<std::vector<T>, T> {
  public:
  VectorIterator(const std::vector<T>& data)
      : data(data) {}

  VectorIterator<T>& operator++() {
    current++;
    return *this;
  }

  T operator*() const {
    if (current < 0 || current > data.size() - 1) {
      return nullptr;
    }
    return this->data[current];
  }

  private:
  std::vector<T> data;
  size_t current = 0;
};
