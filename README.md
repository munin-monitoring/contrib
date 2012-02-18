# This is the repository for all user contributed stuff

## contrib/plugins/ - 3rd-party plugins

**This is usually where you want to begin your journey.**

Here you'll find all the plugins coming from http://exchange.munin-monitoring.org/. 
That web site is for the time being disabled, new updates are done here.

If a dedicated website comes back alive, its plugin backend will be this git repo.

## contrib/templates/ -  3rd-party templates

Feel free to update templates here, or even to create new ones.

Bonus points for mobile-friendly ones :)

Note that the one named `official` is a loose-synced copy of the one in SVN trunk. 
It should serves as a base for small editions that can be resynced in SVN trunk, so for that : 

* don't copy the whole template
* directly edit files in this directory

## contrib/tools/ - 3rd-party tools

Here, you can put just any kind of tool. Please use this directory instead of a random place on the internet. 
It makes things way more easy to search for others.

And, it serves as an incubator of SVN `trunk/contrib` :-)

## Notes to contributors

We like to have ''elementary'' commits (a good rationale is : one per Changelog entry), as it is much easier to manage for reviewing. Debugging is also usually easier that way.

So please **don't** be afraid to make as many commits as needed.
