# Lexbor

Crafting a Browser Engine with Simplicity and Flexibility.

## Description

Lexbor is still in development, but the existing modules are already production-ready.

A set of fast, standards-compliant tools (modules) for working with modern web technologies — HTML parsing, CSS processing, URL handling, and more. These modules are production-ready today and form the foundation of a browser engine in development.

## Features

- **High Performance** — one of the fastest HTML parsers available
- **Standards Compliant** — rigid adherence to [WHATWG](https://whatwg.org/) (HTML, DOM, URL, Encoding) and [W3C](https://www.w3.org/Style/CSS/) (CSS) specifications
- **Modular Architecture** — use only what you need (e.g., just the CSS parser or Encoding module) to keep your application lightweight
- **Zero Dependencies** — written in pure C99, making it easy to assist, build, and embed in any project without dependency hell
- **Production Ready** — heavily tested on over 200 million web pages to ensure stability and correctness

## Available Modules

| Module | Status | Description |
|--------|--------|-------------|
| DOM | ✅ Ready | DOM tree manipulation |
| HTML | ✅ Ready | Full HTML parser |
| CSS | ✅ Ready | CSS parsing, CSSOM, Selectors |
| URL | ✅ Ready | URL parsing |
| Encoding | ✅ Ready | 40+ encodings support |
| Unicode | ✅ Ready | Normalization, IDNA |
| Punycode | ✅ Ready | IDN encode/decode |
| Layout | 🚧 In progress | — |
| Fonts | 🚧 In progress | — |
| and more | 🚧 In progress | — |

https://lexbor.com/modules/.

## Who Uses Lexbor?

- **[PHP](https://www.php.net/)** — DOM/HTML extension (since PHP 8.4), URL extension (since PHP 8.5)
- **[SerpApi](https://serpapi.com/)** — uses Lexbor in production for HTML parsing at scale
- **[Selectolax](https://github.com/rushter/selectolax)** — popular Python library for fast web scraping
- **[Nokolexbor](https://github.com/serpapi/nokolexbor)** — high-performance Nokogiri alternative for Ruby
- **[Nordstjernen](https://github.com/nordstjernen-web/nordstjernen)** - Web browser written entirely in C

[More bindings](#external-bindings-and-wrappers) available for Elixir, Crystal, D, Julia, Erlang.

### HTML Module

* Full conformance with the [HTML5 specification](https://html.spec.whatwg.org/multipage/).
* Manipulation of [elements](https://github.com/lexbor/lexbor/blob/master/examples/lexbor/html/element_create.c) and [attributes](https://github.com/lexbor/lexbor/blob/master/examples/lexbor/html/element_attributes.c): add, change, delete and other.
* Supports fragment parsing (for [innerHTML](https://github.com/lexbor/lexbor/blob/master/examples/lexbor/html/element_innerHTML.c)).
* Supports parsing [by chunks](https://github.com/lexbor/lexbor/blob/master/examples/lexbor/html/document_parse_chunk.c).
* Passes all tree construction tests.
* [Tested](https://github.com/lexbor/warc_test) by 200+ million HTML pages with [ASAN](https://clang.llvm.org/docs/AddressSanitizer.html).
* Two ways to parse HTML: [by Document](https://github.com/lexbor/lexbor/blob/master/examples/lexbor/html/document_parse.c), [by Parser](https://github.com/lexbor/lexbor/blob/master/examples/lexbor/html/parse.c).
* Supports [determining encoding](https://github.com/lexbor/lexbor/blob/master/examples/lexbor/html/encoding.c) by byte stream.
* [Parsing CSS Styles](https://github.com/lexbor/lexbor/tree/master/examples/lexbor/styles) in tag attributes and in the `<style>` tag.

Documentation: https://lexbor.com/modules/html/.

### CSS Module

* Full conformance with the [CSS Syntax](https://drafts.csswg.org/css-syntax-3/) module.
* Supports:
* * [x] [Selectors](https://github.com/lexbor/lexbor/tree/master/examples/lexbor/selectors).
* * [x] [StyleSheet Tree](https://github.com/lexbor/lexbor/tree/master/examples/lexbor/css) (aka CSSOM).
* * [x] and so on.

Documentation: https://lexbor.com/modules/css/.

### Selectors Module

* Search for HTML elements using CSS selectors.

Documentation: https://lexbor.com/modules/selectors/.

### Encoding Module

* Full conformance with the [Encoding specification](https://encoding.spec.whatwg.org/).
* Supports `40 encodings` for encode/decode.
* Supports [single](https://github.com/lexbor/lexbor/blob/master/examples/lexbor/encoding/single/from_to.c) and [buffering](https://github.com/lexbor/lexbor/blob/master/examples/lexbor/encoding/buffer/from_to.c) encode/decode.

Documentation: https://lexbor.com/modules/encoding/.

### URL Module

* Conformance with the [URL specification](https://url.spec.whatwg.org/)
* Support [Unicode ToASCII](https://www.unicode.org/reports/tr46/#ToASCII)

Documentation: https://lexbor.com/modules/url/.

### Punycode Module

* Conformance with the [Punycode specification](https://www.rfc-editor.org/rfc/inline-errata/rfc3492.html).
* Support Encode/Decode.

### Unicode Module

* Unicode Standard Annex [#15](https://www.unicode.org/reports/tr15/).
* * Support Unicode normalization forms: D (NFD), C (NFC), KD (NFKD), KC (NFKC).
* * Support chunks (stream).
* Unicode Technical Standard [#46](https://unicode.org/reports/tr46/).
* * Support Unicode [IDNA Processing](https://www.unicode.org/reports/tr46/#Processing).
* * Support Unicode [ToASCII](https://www.unicode.org/reports/tr46/#ToASCII).
* * Support Unicode [ToUnicode](https://www.unicode.org/reports/tr46/#ToUnicode).

Documentation: https://lexbor.com/modules/unicode/.

## Build and Installation

### Binary packages

Binaries are available for:

* [CentOS](https://lexbor.com/download/#centos) 6, 7, 8
* [Debian](https://lexbor.com/download/#debian) 8, 9, 10, 11
* [Fedora](https://lexbor.com/download/#fedora) 28, 29, 30, 31, 32, 33, 34, 36, 37
* [RHEL](https://lexbor.com/download/#rhel) 7, 8
* [Ubuntu](https://lexbor.com/download/#ubuntu) 14.04, 16.04, 18.04, 18.10, 19.04, 19.10, 20.04, 20.10, 21.04, 22.04

Currently for `x86_64` architecture.
If you need any other architecture, please, write to [support@lexbor.com](mailto:support@lexbor.com).

### vcpkg

For vcpkg users there is a `lexbor` [port](https://github.com/microsoft/vcpkg/tree/master/ports/lexbor) that can be installed via `vcpkg install lexbor` or by adding it to `dependencies` section of your `vcpkg.json` file.

### macOS

#### Homebrew

To install `lexbor` on macOS from Homebrew:

```sh
brew install lexbor
```

#### MacPorts

To install `lexbor` on macOS from MacPorts:

```sh
sudo port install lexbor
```

### Source code

For building and installing Lexbor library from source code, use [CMake](https://cmake.org/) (open-source, cross-platform build system).

```bash
cmake . -DLEXBOR_BUILD_TESTS=ON -DLEXBOR_BUILD_EXAMPLES=ON
make
make test
```

Please, see more information in [documentation](https://lexbor.com/documentation/#source-code).

## Amalgamation

Lexbor can be built as a single-header/single-source amalgamation for easy integration into your project without managing multiple files or dependencies.

The amalgamation combines all selected modules and their dependencies into one `.h` file, making it simple to drop into any C/C++ project.

### Generate Amalgamation

Use the `single.pl` script to generate an amalgamated version:

```bash
# Generate amalgamation with all modules
perl single.pl --all > lexbor_single.h

# Generate amalgamation for specific modules
perl single.pl html css > lexbor_html_css_single.h

# Generate with exported symbols (for dynamic linking)
perl single.pl --with-export-symbols html > lexbor_html_single.h

# Use a different port (default: posix)
perl single.pl --port=windows_nt html > lexbor_html_single.h
```

Once generated, simply include the amalgamated file in your project:

```C
#include "lexbor_single.h"

int
main(void)
{
    /* Your code using Lexbor. */
    return 0;
}
```

Compile without any additional dependencies:

```bash
gcc -o myapp myapp.c lexbor_single.h
```

Documentation: https://lexbor.com/amalgamation/.

## Single or separately

### Single
* liblexbor — this is a single library that includes all modules.

### Separately
* liblexbor-{module name} — libraries for each module.

If you only need an HTML parser, use `liblexbor-html`.

Separate modules may depend on each other.
For example, dependencies for `liblexbor-html`: `liblexbor-core`, `liblexbor-dom`, `liblexbor-tag`, `liblexbor-ns`.

The `liblexbor-html` library already contains all the pointers to the required dependencies. Just include it in the assembly: `gcc program.c -llexbor-html`.

## External Bindings and Wrappers

* [Elixir](https://github.com/dashbitco/lazy_html) Fast HTML parsing and querying. Default HTML engine in Phoenix LiveViewTest (since LiveView 1.1).
* [Elixir](https://git.pleroma.social/pleroma/elixir-libraries/fast_html) binding for the HTML module (since 2.0 version)
* [Erlang](https://hex.pm/packages/lexbor_erl) Fast HTML5 Parser with CSS selectors and DOM manipulation (since 2.6.0 version)
* [Crystal](https://github.com/kostya/lexbor) Fast HTML5 Parser with CSS selectors for Crystal language
* [Python](https://github.com/rushter/selectolax#available-backends) binding for modest and lexbor engines.
* [D](https://github.com/trikko/parserino) Fast HTML5 Parser with CSS selectors for D programming language
* [Ring](https://github.com/ysdragon/ring-html) Fast HTML5 Parser with CSS selectors and DOM manipulation for the Ring programming language.
* [Ruby](https://github.com/serpapi/nokolexbor) Fast HTML5 Parser with both CSS selectors and XPath support.
* [PHP](https://github.com/php/php-src)'s DOM extension uses Lexbor's HTML living standard parser and CSS selector support, starting from PHP 8.4.
* [Julia](https://github.com/MichaelHatherly/Lexbor.jl) binding for the HTML module.

You can create a binding or wrapper for the `lexbor` and place the link here!

## AI Policy

Lexbor draws a clear line between its core library and the surrounding ecosystem.

**Core library** — all source code that compiles into the distributable binary — is written entirely by humans. This is an engineering choice, not an ideological one. Lexbor is a performance-critical, standards-compliant C library where every line of code must reflect a decision its author fully understands and can defend. We use AI tools in other areas of the project and see their value clearly; the core is simply not the right place for them.

AI-assisted tools are welcome and actively used for bindings, WASM builds, benchmarks, documentation, examples, tests, and other supporting work. This section is one such example — it was drafted with AI and reviewed by a human. The em dashes are a giveaway.

**For contributors:** pull requests targeting core library code are expected to be human-authored. The contributor — not an AI model — performed the reasoning, made the design choices, and wrote the code. The code you submit is yours, and you can speak to the intent and correctness of every line. We are not interested in policing workflows, but if a pull request reads like AI did the thinking, it will likely be rejected.

## Documentation

Available on [lexbor.com](https://lexbor.com) in [Documentation](https://lexbor.com/documentation/) section.

## Roadmap

Please, see [roadmap](https://lexbor.com/roadmap/) on [lexbor.com](https://lexbor.com).

## Getting Help

* E-mail [support@lexbor.com](mailto:support@lexbor.com)

## Our Sponsors

[<img src="images/neural-logo.png" alt="goneural.ai" width="320">](https://goneural.ai/) [<img src="images/SerpApi-logo.png" alt="serpapi.com" width="320">](https://serpapi.com/?utm_source=lexbor)

## Sponsorship

You can help sponsor the maintainers of this software through the following organization:
[github.com/sponsors/toxypi](https://github.com/sponsors/toxypi)

## SAST Tools

[PVS-Studio](https://pvs-studio.com/en/pvs-studio/?utm_source=website&utm_medium=github&utm_campaign=open_source) - static analyzer for C, C++, C#, and Java code.

## Status of available distributions

These are third-party distributions; we do not create them. Thank you to the community.

[![Packaging status](https://repology.org/badge/vertical-allrepos/lexbor.svg?exclude_unsupported=1)](https://repology.org/project/lexbor/versions)

## COPYRIGHT AND LICENSE

   Lexbor.

   Copyright 2018-2026 Alexander Borisov

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.


Please, see [LICENSE](https://github.com/lexbor/lexbor/blob/master/LICENSE) file.
