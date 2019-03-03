//--------------------------------------------------------------------------------------------
// Name: NormalizedRange.h
// Author: Jérémi Panneton
// Creation date: February 26th, 2019
//--------------------------------------------------------------------------------------------

#pragma once

template<typename T>
class NormalizedRange
{
public:
	using ValueType = T;

	NormalizedRange(ValueType minValue, ValueType maxValue)
	{
		jassert(minValue < maxValue);
		setMinMax(minValue, maxValue);
	};

	void setMinMax(ValueType minValue, ValueType maxValue)
	{
		m_minValue = minValue;
		m_maxValue = maxValue;
		m_minLogValue = std::log(minValue);
		m_maxLogValue = std::log(maxValue);
	};

	ValueType getMin() const noexcept
	{
		return m_minValue;
	}

	ValueType getMax() const noexcept
	{
		return m_maxValue;
	}

	ValueType getSize() const noexcept
	{
		return getMax() - getMin();
	}

	ValueType getLinearValue(ValueType normalizedPosition) const
	{
		return (normalizedPosition * (m_maxValue - m_minValue)) + m_minValue;
	};

	ValueType getLogValue(ValueType normalizedPosition) const
	{
		return std::exp((normalizedPosition * (m_maxLogValue - m_minLogValue)) + m_minLogValue);
	};

	ValueType getNormalizedLog(ValueType linearValue) const
	{
		if (linearValue <= m_minValue)
		{
			return ValueType(0.0);
		}
		else if (linearValue >= m_maxValue)
		{
			return ValueType(1.0);
		}
		else
		{
			return (std::log(linearValue) - m_minLogValue) / (m_maxLogValue - m_minLogValue);
		};
	};

private:
	ValueType m_minValue = {};
	ValueType m_maxValue = {};
	ValueType m_minLogValue = {};
	ValueType m_maxLogValue = {};
};