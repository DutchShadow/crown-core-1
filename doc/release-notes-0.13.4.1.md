# Crown v0.13.4.1 Release Notes

Crown Core v0.13.4.1 is a minor release and an optional upgrade for anyone
already running a v0.13.4.0 wallet or node. It is required for new users 
to be able to reliably sync a new node or wallet because it uses the new 
seednodes. It is available for download from:

https://crownplatform.com/downloads

Please report bugs through the Help, Guides and Support board at the
Crown forum

https://forum.crownplatform.com/index.php?board=5.0

## How to upgrade

The basic procedure is to shutdown your existing version, replace the 
executable with the new version, and restart. This is true for both wallets
and masternodes/systemnodes. The exact procedure depends on how you setup
the wallet and/or node in the first place. If you use a managed hosting 
service for your node(s) it is probable they will upgrade them for you, 
but you will need to upgrade your wallet. There is no protocol version 
change in this release so if your MN/SN is updated quickly enough it will 
remain active and you won't need to isue a start-alias for it.

## What's new

v0.13.4.1 features new seednode addresses. It also fixes a handful of minor 
bugs:
* -snconf option
* change estimate of initial chainsize
* latest linearize
* masternode/systemnode sync in QT wallet
* updated home URLs
* doxygen logo
* confusing MN/SN readiness status message 
