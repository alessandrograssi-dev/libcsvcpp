# libcsvcpp

Modern C++ wrapper around a legacy C CSV parsing library.

## Purpose

This project explores how to integrate and modernize a legacy C library for use in modern C++ applications, with a focus on clean API design, safe ownership semantics, and build system hygiene.

## Scope

- Wrap a legacy C CSV parser with a modern C++17 interface
- Encapsulate the C implementation as an internal detail
- Use RAII and type-safe abstractions
- Provide a simple, idiomatic C++ API

## Planned Structure
- legacy/ # legacy C library (vendored)
- csvcpp/ # modern C++ wrapper
- examples/ # usage examples
- benchmarks/ # performance experiments

## Build System

The project will use CMake as the primary build system.

## Status

Project scaffold – implementation in progress.