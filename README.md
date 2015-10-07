# Run Meteor from squashfs.

And use overlayfs to install new packages.

---

## Provided

You can [download](../../releases) the `dist.tar.gz` tarball.
It has the following:

The **install** script sets things up, resulting in something comparable to what https://install.meteor.com/ does.
In fact, part of the script is copied from that URL.
This script internally calls `sudo` to do the mounting and to write `/usr/local/bin/meteor`.
**uninstall** undoes that stuff, because I need to do that a lot while developing this.

The **activate** script does the actual mounting.
I don't think you need to call this manually, but if you do,
you have to call it with permission to mount,
and you have to call it from the directory that contains it.
**deactivate** unmounts what *activate* mounts.

When the overlayfs is not mounted, `~/.meteor/meteor` is a symlink to **meteor-wrapper**,
which calls *activate* and then calls the real `meteor`.

**bootstrap.squashfs** is Meteor's bootstrap tarball built into a squashfs image.
It excludes the `package-metadata` directory, which contains a database that changes a lot.
We don't know your UID, so everything in it is owned by root.

**ro**, **rw**, and **work** are the lower-, upper-, and workdirs for an overlayfs that's mounted to `~/.meteor`.
*ro* is an empty mount point for *bootstrap.squashfs*.
*rw* starts out with the `package-metadata` directory from Meteor's bootstrap tarball.
It also has an empty `packages` directory so that Meteor can create new package directories without root access.

## Known issues

### Ownership in squashfs image
The root user owns everything in the squashfs image.
Normally, the user owns everything in `~/.meteor`.
What we have is a set of bandaids that allow Meteor to do some things it needs to do.
This feels like it could break at the slightest disturbance.

### Uncompressed packages.data.db
The `packages.data.db` in `rw/package-metadata` is huge and low-entropy.
It's bigger than the squashfs image.
But the Meteor tool needs to write to it all the time.
What can we do about this?

### $HOME inside sudo
On some systems, `sudo` resets the `$HOME` variable, which breaks the *activate* and *deactivate* scripts.

### Limitations of overlayfs
This whole thing shouldn't even work, because
["Symlinks in /proc/PID/ and /proc/PID/fd which point to a non-directory
object in overlayfs will not contain valid absolute paths, only
relative paths leading up to the filesystem's root."]
(https://www.kernel.org/doc/Documentation/filesystems/overlayfs.txt)
And [that's how Node.js gets its own path](https://github.com/nodejs/node/blob/v0.10.40/deps/uv/src/unix/linux-core.c#L367),
and [that's how Meteor runs your app process](https://github.com/meteor/meteor/blob/9654a7c436b4f65dfa639cc4206fe35a5c46e48c/tools/runners/run-app.js#L232).
Yet somehow, it's working fine on one of my computers.
