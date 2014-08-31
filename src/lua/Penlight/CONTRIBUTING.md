Contributing to Penlight
========================

So you want to contribute to Penlight? Fantastic! Here's a brief overview on
how best to do so.

## What to change

Here's some examples of things you might want to make a pull request for:

* New features
* Bugfixes
* Inefficient blocks of code

If you have a more deeply-rooted problem with how the library is built or some
of the stylistic decisions made in the code, it's best to
[create an issue](https://github.com/stevedonovan/Penlight/issues) before putting
the effort into a pull request. The same goes for new features - it might be
best to check the project's direction, existing pull requests, and currently open
and closed issues first.

## Using Git appropriately
Penlight uses two main branches;

1. `stable` which is the current released version. This version only accepts bugfixes and backward compatible changes/additions. 
1. `master` is the next major version, with new features and probably incompatibilities. Fixes and changes from `stable` will regularly be merged in `master` as well.

Here's how to go about contributing to Penlight

1. [Fork the repository](https://github.com/stevedonovan/Penlight/fork_select) to
your Github account.
2. Create a *topical branch* - a branch whose name is succint but explains what
you're doing, such as _"added-klingon-cloacking-device"_ - either from `master` or `stable` depending on what your changing.
3. Make your changes, committing at logical breaks.
4. Push your branch to your personal account
5. [Create a pull request](https://help.github.com/articles/using-pull-requests)
6. Watch for comments or acceptance

If you wanna be a rockstar;

1. update the [CHANGES.md](https://github.com/stevedonovan/Penlight/blob/master/CHANGES.md) file
1. [add tests](https://github.com/stevedonovan/Penlight/tree/master/tests) that show the defect your fix repairs, or that tests your new feature

Please note - if you want to change multiple things that don't depend on each
other, make sure you check the `master`/`stable` branch back out before making more
changes - that way we can take in each change separately. And against the correct branch.
