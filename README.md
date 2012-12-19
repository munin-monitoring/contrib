This is the repository for all user contributed stuff

# contrib/plugins/ - 3rd-party plugins

**This is usually where you want to begin your journey.**

Here you'll find all the plugins coming from http://exchange.munin-monitoring.org/. 
That web site is for the time being disabled, new updates are done here.

If a dedicated website comes back alive, its plugin backend will be this git repo.

# contrib/templates/ -  3rd-party templates

Feel free to update templates here, or even to create new ones.

Bonus points for mobile-friendly ones :)

Note that the one named `official` is a loose-synced copy of the one in SVN trunk. 
It should serves as a base for small editions that can be resynced in SVN trunk, so for that : 

* don't copy the whole template
* directly edit files in this directory

# contrib/tools/ - 3rd-party tools

Here, you can put just any kind of tool. Please use this directory instead of a random place on the internet. 
It makes things way more easy to search for others.

And, it serves as an incubator of SVN `trunk/contrib` :-)

# contrib/samples/ - 3rd-party examples of configs

This serves as a repository for examples of various configs. You know, the ''learn by example'' way of doing things.

## Notes to contributors

### Commits, Comments & Pull requests

We like to have ''elementary'' commits as it is much easier to manage for reviewing and debugging. 
So please **don't** be afraid to make **as many** commits as needed. Merging many commits is as easy
as merging one, if not easier.

A good rationale is that each commit shall have a one-liner commit comment as its first line. 
Next lines are optional and should only explain the ''why'' it is done this particular way.

On the other side, pull requests can regroup many commits at once.
Just try to explain in the pull comment the ''why'' we should merge it (if it's not obvious).

Tim Pope wrote a [very nice tuto](http://tbaggery.com/2008/04/19/a-note-about-git-commit-messages.html) on making good commit comments.

### Licenses

All the code here is licensed with the same terms as munin itself (GPLv2), unless specified otherwise inside a file.
In all cases the code shall have an OSI-compatible license. Asking for a pull implies that you agree with that fact.

This change was made on Jun 1st 2012. If you wrote some code earlier and you do not agree to the new licensing default, you can  :
- submit a licensing change pull
- submit a removal pull 
