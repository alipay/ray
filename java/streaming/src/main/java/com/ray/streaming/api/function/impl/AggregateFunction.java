package com.ray.streaming.api.function.impl;

import com.ray.streaming.api.function.Function;

/**
 * Aggregate Function.
 * @param <I> In
 * @param <A> Agg
 * @param <O> Out
 */
public interface AggregateFunction<I, A, O> extends Function {

  A createAccumulator();

  void add(I value, A accumulator);

  O getResult(A accumulator);

  A merge(A a, A b);

  void retract(A acc, I value);
}
