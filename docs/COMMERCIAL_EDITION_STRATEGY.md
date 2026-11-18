# LinWin Free and Pro product boundary

## Decision

LinWin will have a complete, public, open-source edition and an optional commercial companion edition:

- **LinWin Free** is maintained in [`sudipdasin/LinWin`](https://github.com/sudipdasin/LinWin), licensed under GPL-3.0-or-later, and remains independently buildable and usable. Community developers can fork it, contribute improvements, and redistribute compliant builds.
- **LinWin Pro** is developed in [`sudipdasin/LinWin-Pro`](https://github.com/sudipdasin/LinWin-Pro), a private repository. The private project keeps an unmodified synchronized snapshot of public LinWin under `public-core/`. Pro-only code, product data, licensed assets, signing/release configuration, and commercial material stay under `proprietary/` or other clearly private paths in that repository.
- A future Pro package adds a **separate executable** that users can install alongside LinWin Free. The free shell must not require a Pro account, network service, license check, or Pro package for its normal operation.

The private repository is a development workspace, not a way to make GPL-covered code proprietary. The mirrored public source retains its existing license and notices. When conveying a combined installer or product bundle, preserve the user's GPL rights in the free program and keep the Pro application a separate work under its own terms.

## Repository layout

```text
Public: sudipdasin/LinWin (GPL-3.0-or-later)
  src/                    LinWin Free shell
  data/                   Free desktop/session resources
  tests/                  Free component and integration tests
  docs/                   Public architecture and extension contract
  packaging/              Free package definitions

Private: sudipdasin/LinWin-Pro (private)
  public-core/            Hourly snapshot of public main, read-only to Pro developers
  proprietary/            Pro-only source and private resources
  pro-tests/              Tests for proprietary components
  pro-packaging/          Optional Pro package and combined installer
  legal/                  Commercial terms and third-party license records
  .github/workflows/      Private synchronization and release workflows
```

Do not copy or stage any private path into the public repository. Public core changes are developed and reviewed in `sudipdasin/LinWin` first; the private sync imports those changes into `public-core/`. Pro developers must not edit the mirrored tree directly. If a Pro need requires a Free API change, propose it publicly and keep the public API useful for non-Pro clients.

## Sync rules

The private workflow fetches public `main`, archives its tracked files into `public-core/`, and records the exact upstream commit SHA in `sync/UPSTREAM_REVISION`. It uses only the private repository's `GITHUB_TOKEN` to write that private repository. It does not push to, create a branch in, or grant permissions on the public repository. Run it hourly and manually before Pro integration builds. Each sync is a snapshot commit and fails if it cannot update the private branch; it never overwrites private paths. Never use a public workflow or public Actions artifact for private source, keys, or assets.

The sync is one-way: public `main` → private `public-core/`. It is not a fork-sync that overwrites the private default branch. Private work remains in its own top-level paths. A failed fetch, archive, or push must stop the workflow without force-pushing or rewriting history.

## Integration boundary

The first Pro releases should be a suite of optional companion applications, not an in-process plugin to LinWin Free. A companion can appear in the normal application launcher through a standard `.desktop` entry. If tighter integration becomes necessary, define and publish a versioned D-Bus interface or similarly documented local IPC contract in the public repository, then keep the client and server in separate processes and address spaces.

The interface must be optional, documented, permission-aware, and independently testable with a free mock client/server. Pro services must not start automatically unless the user enables them. Avoid private shared libraries loaded by the Free process, linking Pro objects into `linwin`, complex shared-memory exchange, private source in public CI, or hard-coded license checks in Free. Free should degrade gracefully when Pro is absent or stopped.

This separation is a product architecture, not a legal safe harbor. Whether particular programs are legally separate depends on their design, communication, and distribution. Have qualified counsel review the actual code, packaging, Qt/dependency licensing, trademarks, privacy/data flows, and terms before shipping paid binaries. This document is engineering guidance, not legal advice.

## Contributor and copyright policy

Public contributions are accepted under GPL-3.0-or-later. The current project does not assume an assignment of contributor copyright or a blanket right to relicense contributions. Do not copy community-authored changes from `public-core/` into `proprietary/` or distribute them under proprietary terms unless every relevant rightsholder has granted explicit permission. Keep a record of authorship, license, third-party code, fonts, icons, and other asset provenance. The maintainer's own copyrights do not grant rights over other contributors' work.

Keep the public issue tracker and pull requests focused on the public Free edition. Do not post private plans that contain trade secrets, credentials, customer data, or non-public assets. Bug reports for the Free edition should remain public and reproducible without Pro.

## Monetization and release policy

Possible Pro value should be added as separately licensed tools or services with their own clear value, such as guided migration, automated backup and restore, curated device profiles, or managed support. These are candidates, not release promises. Validate demand and define privacy/offline behavior before selecting paid features. Do not weaken Free or move bug fixes out of it to create artificial pressure to pay.

For a future combined download, publish separate package manifests and clearly label which files are GPL and which are proprietary. Provide the source and notices required for the GPL-covered parts, never describe the whole bundle as closed source, and offer LinWin Free without purchase or account creation. Release Free and Pro independently so users can install, update, or remove Pro without breaking the free shell.

## Current status

The private `LinWin-Pro` repository has been created. This strategy and its implementation are in progress. No Pro features or paid binaries are part of the current public beta, and no public IPC extension protocol has been implemented yet.
