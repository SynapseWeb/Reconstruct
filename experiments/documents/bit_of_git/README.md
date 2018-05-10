# A Bit of Git


## Overview

"Git" is a tool created by Linus Torvalds to help keep track of the
development of the Linux operating system.

In its simplest form, it's a hidden subdirectory named ".git" that keeps a history
of changes to various files associated with that directory. If I have 3 files in
my project directory, then copies of those files will be stored in the ".git" subdirectory
within that directory. Every time I change one of those files and "commit" the changes,
a new entry will be made inside the ".git" subdirectory with those changes. In a sense,
the ".git" directory is a database storing the history of every version of those files.

If you look inside the ".git" subdirectory, you'll find a number of files and directories
that make up the "git database". You'll find subdirectories (like "branches", "logs", and
"objects"). You'll also find some control files (like "index", "config", and "description").
In general, you don't need to worry about any of that. It's all part of "the database" that
git uses to keep track of the history of all those files.

## Simple Stand Alone Use

Any directory on your computer can be the location for a "git" database related to the
files in that directory (and its subdirectories). Assume you've got a subdirectory named:

::

  ../work_stuff/new_neuron

If you are in that directory and you initialize a git database there, it will create:

::

  ../work_stuff/new_neuron/.git

That subdirectory will contain the entire git repository for that project. You might
have files named: nn.1, nn.2, nn.3, nn.ser and they would all be tracked in that ".git"
subdirectory. You might also create related subdirectories:

::

  ../work_stuff/new_neuron/2018_paper_data
  ../work_stuff/new_neuron/2019_grant_data

All the files in those directories would also be tracked in the same git database:

  ../work_stuff/new_neuron/.git

All their revisions and logs for the entire directory tree (below "new_neuron") would
be stored in that same git database directory.

You could then create another completely separate git database (repository) for a
different project:

::

  ../work_stuff/newer_neuron

Again, git would create a ".git" database (repository) for all the files in that directory:

::

  ../work_stuff/newer_neuron/.git

So you can use git to very easily track virtually anything you're working on in any
directory tree of your computer. It's like your own personal time machine.

## Shared Use

So far, everything has been done on your own local machine. Your git database is a
complete copy of all the changes you've made. In general, that's how git always
works. You'll always have the ability to work without being connected to any
central database. But additionally, git supports merging between your local
database and other people's databases. This is called "pushing" and "pulling"
to a "remote" database. This pushing and pulling can also be done locally on
your own computer or on a shared file system. Git has no inherent notion of a
"central" repository. Everyone's repository is as good as anyone elses. It's
completely decentralized in that respect. You can even put a repository on a
flash drive and push and pull to that flash drive to move it around. The notion
of a "central" location is really more of an agreement between developers than
an inherent part of Git.

With those thoughts in mind, there are many ways to share these distributed
databases with Git. Any computer can be set up as a Git server without using
GitHub or GitLab or anything other than Git itself. Git defines its own protocol,
and any computer set up to serve files with that protocol can be a git server.

Once a computer is set up as a Git server, people with access can "clone" from
that database to make their own local copy. They can then make changes and store
those changes in their own local git databse. If they have permission, they can
then "push" those changes back to the original database and "pull" other people's
changes into their own local database. That's the typical workflow in Git.

## GitHub and GitLab

Up to this point, everything has been pure "Git". All storage, tracking, changing,
logging, committing, pushing, and pulling has all been done with "Git" alone. This
is a completely acceptable way to operate, and it makes the Git database completely
independent of any service provider on the Internet.

But as with everything these days, it's also handy to have a web interface to the
database. That's what services like "GitHub" and "GitLab" provide. They're both
just a pretty interface to the data stored in a "git" repository. They also provide
their own git server to handle the pushing and pulling so users don't have to set
up their own git server to easily share git databases.

GitHub and GitLab (and others) offer free repositories for open source projects
(meaning the data is publicly available to the world). They both also offer paid
services for private repositories. Additionally, the GitLab source code is itself
open source (unlike GitHub) so anyone can set up their own GitLab server for their
own public or private databases.

## Options

There are a number of options for using Git for curating Reconstruct data. One
of the key decision points will be whether there's a need for privacy of the
data. If privacy isn't a concern, then either GitHub or GitLab will work fine
with their free options. If privacy is a concern, then both have paid options
which don't have to be open source. Alternatively, we could set up our own Git
servers (either as simple git servers or as GitLab servers). We already have a
GitLab server set up internally here and we've been talking with our systems
administrators about how to set up a GitLab server either here or in Austin.

