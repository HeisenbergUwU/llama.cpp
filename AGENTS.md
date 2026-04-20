# llama.cpp — Agent Instructions

## Build

**Only CMake works.** The Makefile intentionally errors out:
```
cmake -B build
cmake --build build --config Release
```

Preset builds (`CMakePresets.json`) use Ninja. macOS target uses `arm64-apple-clang-*` presets; Linux uses `x64-linux-gcc-*` presets. Output binaries land in `build/bin/`.

GPU backends are enabled via `GGML_` cache variables (NOT `LLAMA_` — legacy names are deprecated and emit warnings):
- CUDA: `GGML_CUDA=ON`
- Metal: `GGML_METAL=ON`
- Vulkan: `GGML_VULKAN=ON`
- SYCL: `GGML_SYCL=ON`

## Tests

Tests require `LLAMA_BUILD_TESTS=ON` (default when standalone). Run after building:
```
ctest --output-on-failure
```

Tests use CTest labels (default: `main`). Some tests require downloading a model fixture first (`test-download-model`). Several tests are disabled on Windows when building shared libs (they use internal `LLAMA_API` symbols).

Python integration tests: `test-tokenizer-0.sh`, `test-lora-conversion-inference.sh`.

## Code style

- C++17, 4-space indent, line limit 120 chars
- `.clang-format` + `.clang-tidy` (bugprone, readability, clang-analyzer, performance, portability, misc checks with specific exclusions)
- Python: `flake8` via pre-commit; `pyright` via `pyrightconfig.json`; `mypy` with `strict=true`
- Pre-commit hooks: trailing-whitespace, end-of-file-fixer, check-yaml, flake8

## Architecture

| Directory | Role |
|-----------|------|
| `src/` | Core llama library — model loading, inference, KV cache, sampling, vocab, grammar |
| `ggml/` | Subproject: low-level tensor library with GPU backend kernels |
| `common/` | Shared utilities: sampling, chat templating (jinja/PEG), download, ngram cache, logging |
| `tools/` | CLI tools (cli, server, quantize, bench, perplexity, tokenize, etc.) |
| `include/llama.h` | Public C API |
| `models/` | Reference vocab files for tokenizer tests |

Key source files: `src/llama.cpp` (main implementation), `src/llama-model-loader.cpp` (GGUF loading), `src/llama-sampler.cpp` (generation sampling), `src/llama-kv-cache.cpp` (KV cache management).

## Python scripts

Root-level Python scripts for model conversion (`convert_hf_to_gguf.py`, `convert_lora_to_gguf.py`, etc.) use Poetry. Dependencies: `pyproject.toml` references `gguf-py` as a local path dependency. Python type checking with pyright (config: `pyrightconfig.json`, extra paths include `gguf-py`).

## CI

CI runs on PRs touching C/C++/CUDA/Metal/Vulkan/GLSL/WGSL files. See `.github/workflows/build.yml`. Environment vars `GGML_NLOOP`, `GGML_N_THREADS` control test behavior.

## References

- [CONTRIBUTING.md](CONTRIBUTING.md) — project contribution policy (AI usage rules, review process)
- [docs/build.md](docs/build.md) — full build instructions per backend
- [docs/development/HOWTO-add-model.md](docs/development/HOWTO-add-model.md) — adding new model support
- [tools/server/README.md](tools/server/README.md) — server API docs
- [common/jinja/README.md](common/jinja/README.md) — chat template engine
