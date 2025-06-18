#include "MetricSystem.h"
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <iostream>

namespace MetricsSystem {

    // TypedMetricValue template implementations
    template<typename T>
    std::string TypedMetricValue<T>::toString() const {
        if (count_ == 0) {
            return "0";
        }
        
        std::ostringstream oss;
        if constexpr (std::is_floating_point_v<T>) {
            oss << std::fixed << std::setprecision(2) << getValue();
        } else {
            oss << getValue();
        }
        return oss.str();
    }

    template<typename T>
    std::unique_ptr<MetricValue> TypedMetricValue<T>::clone() const {
        auto cloned = std::make_unique<TypedMetricValue<T>>();
        cloned->value_ = value_;
        cloned->count_ = count_;
        return cloned;
    }

    template<typename T>
    void TypedMetricValue<T>::reset() {
        value_ = T{};
        count_ = 0;
    }

    template<typename T>
    void TypedMetricValue<T>::accumulate(const MetricValue& other) {
        const auto* typed_other = dynamic_cast<const TypedMetricValue<T>*>(&other);
        if (!typed_other) {
            throw std::invalid_argument("Cannot accumulate different metric value types");
        }
        
        value_ += typed_other->value_;
        count_ += typed_other->count_;
    }

    // TypedMetric template implementations
    template<typename T>
    void TypedMetric<T>::recordValue(std::unique_ptr<MetricValue> value) {
        auto* typed_value = dynamic_cast<TypedMetricValue<T>*>(value.get());
        if (!typed_value) {
            throw std::invalid_argument("Invalid metric value type for metric: " + name_);
        }

        std::lock_guard<std::mutex> lock(mutex_);
        accumulated_value_ += typed_value->getValue();
        count_++;
    }

    template<typename T>
    void TypedMetric<T>::recordValue(T value) {
        std::lock_guard<std::mutex> lock(mutex_);
        accumulated_value_ += value;
        count_++;
    }

    template<typename T>
    std::unique_ptr<MetricValue> TypedMetric<T>::getAccumulatedValue() const {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (count_ == 0) {
            return std::make_unique<TypedMetricValue<T>>(T{});
        }

        // For different metric types, we might want different aggregation strategies
        if constexpr (std::is_floating_point_v<T>) {
            // For floating point (like CPU usage), return average
            T average = accumulated_value_ / static_cast<T>(count_);
            return std::make_unique<TypedMetricValue<T>>(average);
        } else {
            // For integers (like request counts), return total
            return std::make_unique<TypedMetricValue<T>>(accumulated_value_);
        }
    }

    template<typename T>
    void TypedMetric<T>::reset() {
        std::lock_guard<std::mutex> lock(mutex_);
        accumulated_value_ = T{};
        count_ = 0;
    }

    // Explicit template instantiations for common types
    template class TypedMetricValue<int>;
    template class TypedMetricValue<double>;
    template class TypedMetricValue<float>;
    template class TypedMetricValue<long>;

    template class TypedMetric<int>;
    template class TypedMetric<double>;
    template class TypedMetric<float>;
    template class TypedMetric<long>;

} // namespace MetricsSystem 