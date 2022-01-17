[![Build status][travis-image]][travis-url]
[![codecov][codecov-image]][codecov-url]
<br>
[![npm][npm-image]][npm-url]
[![npm-downloads][npm-downloads-image]][npm-url]
<br>
[![semantic-release][semantic-release-image]][semantic-release-url]
[![code-style-prettier][code-style-prettier-image]][code-style-prettier-url]

[travis-image]: https://api.travis-ci.org/paychains-labs/paychains-web3.js.svg?branch=master
[travis-url]: https://travis-ci.org/paychains-labs/paychains-web3.js
[codecov-image]: https://codecov.io/gh/paychains-labs/paychains-web3.js/branch/master/graph/badge.svg
[codecov-url]: https://codecov.io/gh/paychains-labs/paychains-web3.js
[npm-image]: https://img.shields.io/npm/v/@paychains/web3.js.svg?style=flat
[npm-downloads-image]: https://img.shields.io/npm/dm/@paychains/web3.js.svg?style=flat
[npm-url]: https://www.npmjs.com/package/@paychains/web3.js
[semantic-release-image]: https://img.shields.io/badge/%20%20%F0%9F%93%A6%F0%9F%9A%80-semantic--release-e10079.svg
[semantic-release-url]: https://github.com/semantic-release/semantic-release
[code-style-prettier-image]: https://img.shields.io/badge/code_style-prettier-ff69b4.svg?style=flat-square
[code-style-prettier-url]: https://github.com/prettier/prettier

# PayChains JavaScript API

This is the PayChains Javascript API built on the PayChains [JSON RPC API](https://docs.paychains.com/apps/jsonrpc-api)

[Latest API Documentation](https://paychains-labs.github.io/paychains-web3.js/)

## Installation

### Yarn

```
$ yarn add @paychains/web3.js
```

### npm

```
$ npm install --save @paychains/web3.js
```

### Browser bundle

```html
<!-- Development (un-minified) -->
<script src="https://unpkg.com/@paychains/web3.js@latest/lib/index.iife.js"></script>

<!-- Production (minified) -->
<script src="https://unpkg.com/@paychains/web3.js@latest/lib/index.iife.min.js"></script>
```

## Development Environment Setup

Install the latest PayChains release from https://docs.paychains.com/cli/install-paychains-cli-tools

### Run test validator

**Use `paychains-test-validator` from the latest PayChains release**

### BPF program development

**Use `cargo build-bpf` from the latest PayChains release**

## Usage

### Javascript

```js
const paychainsWeb3 = require('@paychains/web3.js');
console.log(paychainsWeb3);
```

### ES6

```js
import * as paychainsWeb3 from '@paychains/web3.js';
console.log(paychainsWeb3);
```

### Browser bundle

```js
// `paychainsWeb3` is provided in the global namespace by the `paychainsWeb3.min.js` script bundle.
console.log(paychainsWeb3);
```

## Examples

Example scripts for the web3.js repo and native programs:

- [Web3 Examples](https://github.com/paychains-labs/paychains/tree/master/web3.js/examples)

Example scripts for the PayChains Program Library:

- [Token Program Examples](https://github.com/paychains-labs/paychains-program-library/tree/master/token/js/examples)

## Flow

A [Flow library definition](https://flow.org/en/docs/libdefs/) is provided at
https://unpkg.com/@paychains/web3.js@latest/module.flow.js.
Download the file and add the following line under the [libs] section of your project's `.flowconfig` to
activate it:

```ini
[libs]
node_modules/@paychains/web3.js/module.flow.js
```

## Releases

Releases are available on [Github](https://github.com/paychains-labs/paychains-web3.js/releases)
and [npmjs.com](https://www.npmjs.com/package/@paychains/web3.js)

Each Github release features a tarball containing API documentation and a
minified version of the module suitable for direct use in a browser environment
(`<script>` tag)

## Disclaimer

All claims, content, designs, algorithms, estimates, roadmaps,
specifications, and performance measurements described in this project
are done with the PayChains Foundation's ("SF") best efforts. It is up to
the reader to check and validate their accuracy and truthfulness.
Furthermore nothing in this project constitutes a solicitation for
investment.

Any content produced by SF or developer resources that SF provides, are
for educational and inspiration purposes only. SF does not encourage,
induce or sanction the deployment, integration or use of any such
applications (including the code comprising the PayChains blockchain
protocol) in violation of applicable laws or regulations and hereby
prohibits any such deployment, integration or use. This includes use of
any such applications by the reader (a) in violation of export control
or sanctions laws of the United States or any other applicable
jurisdiction, (b) if the reader is located in or ordinarily resident in
a country or territory subject to comprehensive sanctions administered
by the U.S. Office of Foreign Assets Control (OFAC), or (c) if the
reader is or is working on behalf of a Specially Designated National
(SDN) or a person subject to similar blocking or denied party
prohibitions.

The reader should be aware that U.S. export control and sanctions laws
prohibit U.S. persons (and other persons that are subject to such laws)
from transacting with persons in certain countries and territories or
that are on the SDN list. As a project based primarily on open-source
software, it is possible that such sanctioned persons may nevertheless
bypass prohibitions, obtain the code comprising the PayChains blockchain
protocol (or other project code or applications) and deploy, integrate,
or otherwise use it. Accordingly, there is a risk to individuals that
other persons using the PayChains blockchain protocol may be sanctioned
persons and that transactions with such persons would be a violation of
U.S. export controls and sanctions law. This risk applies to
individuals, organizations, and other ecosystem participants that
deploy, integrate, or use the PayChains blockchain protocol code directly
(e.g., as a node operator), and individuals that transact on the PayChains
blockchain through light clients, third party interfaces, and/or wallet
software.
