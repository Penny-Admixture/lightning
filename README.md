# peercoin c-lightning: A specification compliant Lightning Network implementation in C

c-lightning is a lighweight, highly customizable and [standard compliant][std] implementation of the Lightning Network protocol.

* [Getting Started](#getting-started)
    * [Installation](#installation)
    * [Starting lightningd](#starting-lightningd)
    * [Using the JSON-RPC Interface](#using-the-json-rpc-interface)
    * [Care And Feeding Of Your New Lightning Node](#care-and-feeding-of-your-new-lightning-node)
    * [Opening A Channel](#opening-a-channel)
	* [Sending and Receiving Payments](#sending-and-receiving-payments)
	* [Configuration File](#configuration-file)
* [Further Information](#further-information)
    * [Pruning](#pruning)
	* [Developers](#developers)

## Project Status

Based on v0.7.2, minimal peercoin changes are implemented.

## Getting Started

c-lightning only works on Linux and Mac OS, and requires a locally (or remotely) running `peercoind` (version 0.8 or above) that is fully caught up with the network you're testing on.

### Installation

There are 4 supported installation options:

 - Installation from the [Ubuntu PPA][ppa]
 - Installation of a pre-compiled binary from the [release page][releases] on Github
 - Using one of the [provided docker images][dockerhub] on the Docker Hub
 - Compiling the source code yourself as described in the [installation documentation](doc/INSTALL.md).

### Starting `lightningd`

If you want to experiment with `lightningd`, there's a script to set
up a `peercoind` regtest test network of two local lightning nodes,
which provides a convenient `start_ln` helper:

```bash
. contrib/startup_regtest.sh
```

To test with real peercoin,  you will need to have a local `peercoind` node running:

```bash
peercoind -daemon -testnet
```

Wait until `peercoind` has synchronized with the network.

Make sure that you do not have `walletbroadcast=0` in your `~/.peercoin/peercoin.conf`, or you may run into trouble.
Notice that running `lightningd` against a pruned node may cause some issues if not managed carefully, see [below](#pruning) for more information.

You can start `lightningd` with the following command:

```bash
lightningd --network=peercoin --log-level=debug
```

This creates a `.lightning/` subdirectory in your home directory: see `man -l doc/lightningd.8`.

### Using The JSON-RPC Interface

c-lightning exposes a [JSON-RPC 2.0][jsonrpcspec] interface over a Unix Domain socket; the `lightning-cli` tool can be used to access it, or there is a [python client library](contrib/pylightning).

You can use `lightning-cli help` to print a table of RPC methods; `lightning-cli help <command>`
will offer specific information on that command.

Useful commands:

* [newaddr](doc/lightning-newaddr.7.md): get a peercoin address to deposit funds into your lightning node.
* [listfunds](doc/lightning-listfunds.7.md): see where your funds are.
* [connect](doc/lightning-connect.7.md): connect to another lightning node.
* [fundchannel](doc/lightning-fundchannel.7.md): create a channel to another connected node.
* [invoice](doc/lightning-invoice.7.md): create an invoice to get paid by another node.
* [pay](doc/lightning-pay.7.md): pay someone else's invoice.
* [plugin](doc/lightning-plugin.7.md): commands to control extensions.

### Care And Feeding Of Your New Lightning Node

Once you've started for the first time, there's a script called
`contrib/bootstrap-node.sh` which will connect you to other nodes on
the lightning network.

There are also numerous plugins available for c-lightning which add
capabilities: in particular there's a collection at:

	https://github.com/lightningd/plugins

Including [helpme][helpme-github] which guides you through setting up
your first channels and customizing your node.

You can also chat to other users at [#c-lightning @ freenode.net][irc2];
we are always happy to help you get started!


### Opening A Channel

First you need to transfer some funds to `lightningd` so that it can
open a channel:

```bash
# Returns an address <address>
lightning-cli newaddr

# Returns a transaction id <txid>
peercoin-cli sendtoaddress <address> <amount_in_peercoins>
```

`lightningd` will register the funds once the transaction is confirmed.

You may need to generate a p2sh-segwit address if the faucet does not support bech32:

```bash
# Return a p2sh-segwit address
lightning-cli newaddr p2sh-segwit
```

Confirm `lightningd` got funds by:

```bash
# Returns an array of on-chain funds.
lightning-cli listfunds
```

Once `lightningd` has funds, we can connect to a node and open a channel.
Let's assume the **remote** node is accepting connections at `<ip>`
(and optional `<port>`, if not 9735) and has the node ID `<node_id>`:

```bash
lightning-cli connect <node_id> <ip> [<port>]
lightning-cli fundchannel <node_id> <amount_in_satoshis>
```

This opens a connection and, on top of that connection, then opens
a channel.
The funding transaction needs 3 confirmation in order for the channel to be usable, and 6 to be announced for others to use.
You can check the status of the channel using `lightning-cli listpeers`, which after 3 confirmations (1 on testnet) should say that `state` is `CHANNELD_NORMAL`; after 6 confirmations you can use `lightning-cli listchannels` to verify that the `public` field is now `true`.

### Sending and Receiving Payments

Payments in Lightning are invoice based.
The recipient creates an invoice with the expected `<amount>` in
millisatoshi (or `"any"` for a donation), a unique `<label>` and a
`<description>` the payer will see:

```bash
lightning-cli invoice <amount> <label> <description>
```

This returns some internal details, and a standard invoice string called `bolt11` (named after the [BOLT #11 lightning spec][BOLT11]).

[BOLT11]: https://github.com/lightningnetwork/lightning-rfc/blob/master/11-payment-encoding.md

The sender can feed this `bolt11` string to the `decodepay` command to see what it is, and pay it simply using the `pay` command:

```bash
lightning-cli pay <bolt11>
```

Note that there are lower-level interfaces (and more options to these
interfaces) for more sophisticated use.

## Configuration File

`lightningd` can be configured either by passing options via the command line, or via a configuration file.
Command line options will always override the values in the configuration file.

To use a configuration file, create a file named `config` within your lightning directory
(eg. `~/.lightning/config`).  See `man -l doc/lightningd-config.5`.

## Further information

### Developers

Developers wishing to contribute should start with the developer guide [here](doc/HACKING.md).
You should also configure with `--enable-developer` to get additional checks and options.

[std]: https://github.com/lightningnetwork/lightning-rfc
[travis-ci]: https://travis-ci.org/peercoin/lightning.svg?branch=peercoin
[travis-ci-link]: https://travis-ci.org/peercoin/lightning
[prs]: https://img.shields.io/badge/PRs-welcome-brightgreen.svg?style=flat
[prs-link]: http://makeapullrequest.com
[IRC]: https://img.shields.io/badge/chat-on%20freenode-brightgreen.svg
[IRC-link]: https://webchat.freenode.net/?channels=c-lightning
[irc1]: http://webchat.freenode.net/?channels=%23lightning-dev
[irc2]: http://webchat.freenode.net/?channels=%23c-lightning
[ml1]: https://lists.ozlabs.org/listinfo/c-lightning
[ml2]: https://lists.linuxfoundation.org/mailman/listinfo/lightning-dev
[docs]: https://lightning.readthedocs.org
[ppa]: https://launchpad.net/~lightningnetwork/+archive/ubuntu/ppa
[releases]: https://github.com/peercoin/lightning/releases
[dockerhub]: https://hub.docker.com/r/peercoin/lightningd/
[jsonrpcspec]: https://www.jsonrpc.org/specification
[helpme-github]: https://github.com/lightningd/plugins/tree/master/helpme
