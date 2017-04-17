#!/usr/bin/python
# -*- coding: utf-8 -*-

""" 
A munin plugin that prints archive and their upgradable packets

TODO: make it usable and readable as commandline tool
 • (-i) interaktiv
   NICETOHAVE
TODO: separate into 2 graphs
 • how old is my deb installation
   sorting a packet to the oldest archive
   sorting a packet to the newest archive
   (WONTFIX unless someone asks for)

TODO: 
 • addinge alternative names for archives "stable -> squeeze"
TODO: add gray as 
      foo.colour 000000
      to 'now', '', '', '', '', 'Debian dpkg status file'
TODO: update only if system was updated (aptitutde update has been run)
      • check modification date of /var/cache/apt/pkgcache.bin
      • cache file must not be older than mod_date of pkgcache.bin + X
TODO: shorten ext_info with getShortestConfigOfOptions 
TODO: check whether cachefile matches the config 
      • i have no clever idea to do this without 100 lines of code
BUG: If a package will be upgraded, and brings in new dependencies, 
     these new deps will not be counted. WONTFIX
"""
import sys
import argparse
import apt_pkg
from apt.progress.base import OpProgress
from time import time, strftime
import os 
import StringIO
import string
import re
from collections  import defaultdict, namedtuple
from types import StringTypes, TupleType, DictType, ListType, BooleanType

class EnvironmentConfigBroken(Exception): pass

# print environmental things 
# for k,v in os.environ.iteritems(): print >> sys.stderr, "%r : %r" % (k,v)

def getEnv(name, default=None, cast=None):
    """
        function to get Environmentvars, cast them and setting defaults if they aren't
        getEnv('USER', default='nouser') # 'HomerS'
        getEnv('WINDOWID', cast=int) # 44040201
    """
    try:
        var = os.environ[name]
        if cast is not None:
            var = cast(var)
    except KeyError:
        # environment does not have this var
        var = default
    except:
        # now probably the cast went wrong
        print >> sys.stderr, "for environment variable %r, %r is no valid value"%(name, var)
        var = default
    return var

MAX_LIST_SIZE_EXT_INFO = getEnv('MAX_LIST_SIZE_EXT_INFO', default=50, cast=int)
""" Packagelists to this size are printed as extra Information to munin """

STATE_DIR = getEnv('MUNIN_PLUGSTATE', default='.')
CACHE_FILE = os.path.join(STATE_DIR, "deb_packages.state")
""" 
   There is no need to execute this script every 5 minutes.
   The Results are put to this file, next munin-run can read from it
   CACHE_FILE is usually /var/lib/munin/plugin-state/debian_packages.state
"""

CACHE_FILE_MAX_AGE = getEnv('CACHE_FILE_MAX_AGE', default=3540, cast=int)
""" 
   Age in seconds an $CACHE_FILE can be. If it is older, the script updates
"""

def Property(func):
    return property(**func())

class Apt(object):
    """
        lazy helperclass i need in this statisticprogram, which have alle the apt_pkg stuff
    """

    def __init__(self):
        # init packagesystem
        apt_pkg.init_config()
        apt_pkg.init_system()
        # NullProgress : we do not want progress info in munin plugin
        # documented None did not worked
        self._cache = None
        self._depcache = None
        self._installedPackages = None
        self._upgradablePackages = None

    @Property
    def cache():
        doc = "apt_pkg.Cache instance, lazy instantiated"
        def fget(self):
            class NullProgress(OpProgress):
                """ used for do not giving any progress info, 
                    while doing apt things used, cause documented 
                    use of None as OpProgress did not worked in 
                    python-apt 0.7
                """
                def __init__(self):
                    self.op=''
                    self.percent=0
                    self.subop=''

                def done(self):
                    pass

                def update(*args,**kwords):
                    pass

            if self._cache is None: 
                self._cache = apt_pkg.Cache(NullProgress()) 
            return self._cache
        return locals()

    @Property
    def depcache():
        doc = "apt_pkg.DepCache object"

        def fget(self):
            if self._depcache is None: 
                self._depcache = apt_pkg.DepCache(self.cache)
            return self._depcache

        return locals()

    @Property
    def installedPackages():
        doc = """apt_pkg.PackageList with installed Packages
                 it is a simple ListType with Elements of apt_pkg.Package
              """

        def fget(self):
            """ returns a apt_pkg.PackageList with installed Packages
                it is a simple ListType with Elements of apt_pkg.Package
            """
            if self._installedPackages is None:
                self._installedPackages = []
                for p in self.cache.packages:
                    if not ( p.current_state == apt_pkg.CURSTATE_NOT_INSTALLED or
                             p.current_state == apt_pkg.CURSTATE_CONFIG_FILES ):
                        self._installedPackages.append(p)
            return self._installedPackages

        return locals()

    @Property
    def upgradablePackages():

        doc = """apt_pkg.PackageList with Packages that are upgradable
                 it is a simple ListType with Elements of apt_pkg.Package
              """

        def fget(self):
            if self._upgradablePackages is None:
                self._upgradablePackages = []
                for p in self.installedPackages:
                    if self.depcache.is_upgradable(p):
                        self._upgradablePackages.append(p)
            return self._upgradablePackages

        return locals()

apt = Apt()
""" global instance of apt data, used here

        apt.cache
        apt.depcache
        apt.installedPackages
        apt.upgradablePackages

    initialisation is lazy 
"""

def weightOfPackageFile(detail_tuple, option_tuple):
    """
        calculates a weight, you can sort with
        if detail_tuple is: ['label', 'archive']
           option_tuple is: ['Debian', 'unstable']
        it calculates
            sortDict['label']['Debian'] * multiplierDict['label']
          + sortDict['archive']['unstable'] * multiplierDict['archive']
          = 10 * 10**4 + 50 * 10**8
          = 5000100000
    """
    val = 0L
    for option, detail in zip(option_tuple, detail_tuple):
        optionValue = PackageStat.sortDict[option][detail]
        val += optionValue * PackageStat.multiplierDict[option]
    return val

def Tree():
    """ Tree type generator
        you can put data at the end of a twig
        a = Tree()
        a['a']['b']['c'] # creates the tree of depth 3
        a['a']['b']['d'] # creates another twig of the tree
                c
        a — b <
                d
    """
    return TreeTwig(Tree)

class TreeTwig(defaultdict):
    def __init__(self, defaultFactory):
        super(TreeTwig, self).__init__(defaultFactory) 

    def printAsTree(self, indent=0):
        for k, tree in self.iteritems():
            print "  " * indent, repr(k)
            if isinstance(tree, TreeTwig):
                printTree(tree, indent+1)
            else:
                print tree

    def printAsLine(self):
        print self.asLine()

    def asLine(self):
        values = ""
        for key, residue in self.iteritems():
            if residue:
                values += " %r" % key
                if isinstance(residue, TreeTwig):
                    if len(residue) == 1:
                        values += " - %s" % residue.asLine()
                    else:
                        values += "(%s)" % residue.asLine()
                else:
                    values += "(%s)" % residue
            else:
                values += " %r," % key
        return values.strip(' ,')


def getShortestConfigOfOptions(optionList = ['label', 'archive', 'site']):
    """ 
        tries to find the order to print a tree of the optionList
        with the local repositories with the shortest line 
        possible options are:
          'component'
          'label'
          'site'
          'archive'
          'origin' 
          'architecture' 
        Architecture values are usually the same and can be ignored.

        tells you which representation of a tree as line is shortest.
        Is needed to say which ext.info line would be the shortest
        to write the shortest readable output.
    """
    l = optionList # just because l is much shorter
    
    # creating possible iterations
    fieldCount = len(optionList)
    if fieldCount == 1:
        selection = l
    elif fieldCount == 2:
        selection = [(x,y) 
                     for x in l 
                     for y in l if x!=y ]
    elif fieldCount == 3:
        selection = [(x,y,z) 
                     for x in l 
                     for y in l if x!=y 
                     for z in l if z!=y and z!=x]
    else:
        raise Exception("NotImplemented for size %s" % fieldCount)

    # creating OptionsTree, and measuring the length of it on a line
    # for every iteration
    d = {}
    for keys in selection:
        d[keys] = len( getOptionsTree(apt.cache, keys).asLine() )

    # finding the shortest variant
    r = min( d.items(), key=lambda x: x[1] )

    return list(r[0]), r[1]
    
def getOptionsTree(cache, keys=None):
    """
    t = getOptionsTree(cache, ['archive', 'site', 'label'])
    generates ad dict of dict of sets like:
    ...
    it tells you:
    ...
    """
    t = Tree()
    for f in cache.file_list:
        # ignoring translation indexes ...
        if f.index_type != 'Debian Package Index' and f.index_type !='Debian dpkg status file':
            continue
        # ignoring files with 0 size
        if f.size == 0L:
            continue
        # creating default dict in case of secondary_options are empty
        d = t
        for key in keys:
            if not key:
                print f
            dKey = f.__getattribute__(key)
            d = d[dKey]
    return t

def createKey(key, file):
    """
    createKey( (archive, origin), apt.pkg_file)
    returns ('unstable', 'Debian')
    """
    if type(key) in StringTypes:
        return file.__getattribute__(key)
    elif type(key) in (TupleType, ListType): 
        nKey = tuple()
        for pKey in key:
            nKey = nKey.__add__((file.__getattribute__(pKey),))
        return nKey
    else:
        raise Exception("Not implemented for keytype %s" % type(key)) 

def getOptionsTree2(cache, primary=None, secondary=None):
    """ 
    primary muss ein iterable oder StringType sein
    secondary muss iterable oder StringType sein
    t1 = getOptionsTree2(apt.cache, 'origin', ['site', 'archive'])
    t2 = getOptionsTree2(apt.cache, ['origin', 'archive'], ['site', 'label'])
    """


    if type(secondary) in StringTypes:
        secondary = [secondary]
    if type(primary) in StringTypes:
        primary = [primary]

    t = Tree()
    for file in cache.file_list:
        # ignoring translation indexes ...
        if file.index_type not in ['Debian Package Index', 'Debian dpkg status file']:
            continue
        # ignoring files with 0 size
        if file.size == 0L:
            continue

        # key to first Dict in Tree is a tuple
        pKey = createKey(primary, file)
        d = t[pKey]
        if secondary is not None:
            # for no, sKey in enumerate(secondary):
            #     dKey = file.__getattribute__(sKey)
            #     if no < len(secondary)-1:
            #         d = d[dKey]
            # if isinstance(d[dKey], DictType):
            #     d[dKey] = []
            # d[dKey].append(file)

            for sKey in secondary:
                dKey = file.__getattribute__(sKey)
                d = d[dKey]
    return t
    
#def getAttributeSet(iterable, attribute):
#    return set(f.__getattribute__(attribute) for f in iterable)
#
#def getOrigins(cache):
#    return getAttributeSet(cache.file_list, 'origin') 
#
#def getArchives(cache):
#    return getAttributeSet(cache.file_list, 'archive') 
#
#def getComponents(cache):
#    return getAttributeSet(cache.file_list, 'component') 
#
#def getLabels(cache):
#    return getAttributeSet(cache.file_list, 'label') 
#
#def getSites(cache):
#    return getAttributeSet(cache.file_list, 'site') 
#

class PackageStat(defaultdict):
    """ defaultdict with Tuple Keys of (label,archive) containing lists of ArchiveFiles
        {('Debian Backports', 'squeeze-backports'): [...]
         ('The Opera web browser', 'oldstable'): [...]
         ('Debian', 'unstable'): [...]}
        with some abilities to print output munin likes
    """

    sortDict = { 'label': defaultdict(   lambda :  20, 
                                       {'Debian':  90, 
                                        ''      :  1,
                                        'Debian Security' : 90,
                                        'Debian Backports': 90}),
                 'archive': defaultdict( lambda :  5,
                                { 'now':                0, 
                                  'experimental':      10,
                                  'unstable':          50, 
                                  'sid':               50, 
                                  'testing':           70,
                                  'wheezy':            70,
                                  'squeeze-backports': 80,
                                  'stable-backports':  80,
                                  'proposed-updates':  84,
                                  'stable-updates':    85,
                                  'stable':            90,
                                  'squeeze':           90,
                                  'oldstable':         95,
                                  'lenny':             95, } ),
                 'site': defaultdict( lambda :  5, { }),
                 'origin': defaultdict( lambda :  5, { 'Debian' : 90, }),
                 'component': defaultdict( lambda :  5, {
                                  'non-free': 10,
                                  'contrib' : 50,
                                  'main'    : 90, }),
    }
    """
        Values to sort options (label, archive, origin ...)
        (0..99) is allowed. 
        (this is needed for other graphs to calc aggregated weights)
        higher is more older and more official or better 
    """

    dpkgStatusValue = { 'site': '', 'origin': '', 'label': '', 'component': '', 'archive': 'now' }
    """ a dict to recognize options that coming from 'Debian dpkg status file' """

    viewSet = set(['label', 'archive', 'origin', 'site', 'component'])

    multiplierDict = { 'label'     : 10**8,
                       'archive'   : 10**4,
                       'site'      : 10**0,
                       'origin'    : 10**6,
                       'component' : 10**2,
    }
    """
        Dict that stores multipliers 
        to compile a sorting value for each archivefile
    """

    def weight(self, detail_tuple):
        return weightOfPackageFile(detail_tuple=detail_tuple, option_tuple=tuple(self.option))

    def __init__(self, packetHandler, apt=apt, sortBy=None, extInfo=None, includeNow=True, *args, **kwargs):
        assert isinstance(packetHandler, PacketHandler)
        self.packetHandler = packetHandler
        self.apt = apt
        self.option = sortBy if sortBy is not None else ['label', 'archive']
        optionsMentionedInExtInfo = extInfo if extInfo is not None else list(self.viewSet - set(self.option))
        self.options = getOptionsTree2(apt.cache, self.option, optionsMentionedInExtInfo)
        self.options_sorted = self._sorted(self.options.items())
        super(PackageStat, self).__init__(lambda: [], *args, **kwargs)

    translationTable = string.maketrans(' -.', '___')
    """ chars that must not exist in a munin system name"""

    @classmethod
    def generate_rrd_name_from(cls, string):
         return string.translate(cls.translationTable)

    def _sorted(self, key_value_pairs):
         return sorted(key_value_pairs, key=lambda(x): self.weight(x[0]), reverse=True)

    @classmethod
    def generate_rrd_name_from(cls, keyTuple):
        assert isinstance(keyTuple, TupleType) or isinstance(keyTuple, ListType)
        # we have to check, whether all tuple-elements have values
        l = []
        for key in keyTuple:
            key = key if key else "local"
            l.append(key)
        return string.join(l).lower().translate(cls.translationTable)

    def addPackage(self, sourceFile, package):
        if self.packetHandler.decider(package):
            self.packetHandler.adder(package, self)
            
    @classmethod
    def configD(cls, key, value):
        i = { 'rrdName': cls.generate_rrd_name_from(key),
              'options': string.join(key,'/'),
              'info'   : "from %r" % value.asLine() }
        return i

    def configHead(self):
        d = { 'graphName': "packages_"+ self.generate_rrd_name_from(self.option),
              'option': string.join(self.option, '/'),
              'type' : self.packetHandler.type
            }
        return "\n"\
               "multigraph {graphName}_{type}\n"\
               "graph_title {type} Debian packages sorted by {option}\n"\
               "graph_info {type} Debian packages sorted by {option} of its repository\n"\
               "graph_category security\n"\
               "graph_vlabel packages".format(**d)

    def printConfig(self):
        print self.configHead()
        for options, item in self.options_sorted:
            if not self.packetHandler.includeNow and self.optionIsDpkgStatus(details=options):
                 continue
            i = self.configD(options, item)
            print "{rrdName}.label {options}".format(**i)
            print "{rrdName}.info {info}".format(**i)
            print "{rrdName}.draw AREASTACK".format(**i)

    def optionIsDpkgStatus(self, details, options=None):
        """ 
            give it details and options and it tells you whether the datails looks like they come from 
            a 'Debian dpkg status file'.
        """
        # setting defaults
        if options is None:
            options = self.option
        assert type(details) in (TupleType, ListType), 'details must be tuple or list not %r' % type(details)
        assert type(options) in (TupleType, ListType), 'options must be tuple or list not %r' % type(details)
        assert len(details) == len(options)
        isNow = True
        for det, opt in zip(details, options):
            isNow &= self.dpkgStatusValue[opt] == det
        return isNow

    def printValues(self):
        print "\nmultigraph packages_{option}_{type}".format(option=self.generate_rrd_name_from(self.option), 
                                                             type=self.packetHandler.type)
        for options, item in self.options_sorted:
            if not self.packetHandler.includeNow and self.optionIsDpkgStatus(details=options):
                 continue
            i = self.configD(options, item)
            i['value'] = len(self.get(options, []))
            print "{rrdName}.value {value}".format(**i)
            self._printExtInfoPackageList(options)

    def _printExtInfoPackageList(self, options):
        rrdName = self.generate_rrd_name_from(options)
        packageList = self[options]
        packageCount = len( packageList )
        if 0 < packageCount <= MAX_LIST_SIZE_EXT_INFO:
            print "%s.extinfo " % rrdName,
            for item in packageList:
                print self.packetHandler.extInfoItemString.format(i=item),
            print

packetHandlerD = {}
""" Dictionary for PacketHandlerclasses with its 'type'-key """

class PacketHandler(object):
    """
    Baseclass, that represents the Interface which is used 
    """

    type = None
    includeNow = None
    extInfoItemString = None

    def __init__(self, apt):
        self.apt = apt

    def decider(self, package, *args, **kwords):
        """
        Function works as decider
        if it returns True, the package is added
        if it returns False, the package is not added
        """
        pass

    def adder(self, package, packageStat, *args, **kwords):
        """
        take the package and add it tho the packageStat dictionary in defined way
        """
        pass

    @classmethod
    def keyOf(cls, pFile):
        """
        calculates the weight of a apt_pkg.PackageFile
        """
        options = ('origin', 'site', 'archive', 'component', 'label')
        details = tuple()
        for option in options:
            details = details.__add__((pFile.__getattribute__(option),))
        return weightOfPackageFile(details, options)

class PacketHandlerUpgradable(PacketHandler):
    
    type='upgradable'
    includeNow = False
    extInfoItemString = "  {i[0].name} <{i[1]} -> {i[2]}>"

    def decider(self, package, *args, **kwords):
        return self.apt.depcache.is_upgradable(package)

    def adder(self, package, packageStat, *args, **kwords):
        options = tuple(packageStat.option)
        candidateP = self.apt.depcache.get_candidate_ver(package)
        candidateFile = max(candidateP.file_list, key=lambda f: self.keyOf(f[0]) )[0]
        keys = createKey(options, candidateFile)
        # this item (as i) is used for input in extInfoItemString
        item = (package, package.current_ver.ver_str, candidateP.ver_str)
        packageStat[keys].append(item)

# registering PackageHandler for Usage
packetHandlerD[PacketHandlerUpgradable.type] = PacketHandlerUpgradable

class PacketHandlerInstalled(PacketHandler):
    type = 'installed'
    includeNow = True
    extInfoItemString = " {i.name}"

    def decider(self, package, *args, **kwords):
        # this function is called with each installed package
        return True

    def adder(self, package, packageStat, *args, **kwords):
        options = tuple(packageStat.option)
        candidateP = self.apt.depcache.get_candidate_ver(package)
        candidateFile = max(candidateP.file_list, key=lambda f: self.keyOf(f[0]) )[0]
        keys = createKey(options, candidateFile)
        # this item (as i) is used for input in extInfoItemString
        item = package
        packageStat[keys].append(item)
        
# registering PackageHandler for Usage
packetHandlerD[PacketHandlerInstalled.type] = PacketHandlerInstalled

class Munin(object):

    def __init__(self, commandLineArgs=None):
        self.commandLineArgs = commandLineArgs
        self.argParser = self._argParser()
        self.executionMatrix = { 
            'config': self.config,
            'run'   : self.run,
            'autoconf' : self.autoconf,
        }
        self.envConfig = self._envParser()
        self._envValidater()
        # print >> sys.stderr, self.envConfig
        self.statL = []
        if self.envConfig:
            for config in self.envConfig:
                packetHandler = packetHandlerD[config['type']](apt)
                packageStat = PackageStat(apt=apt,
                                                    packetHandler = packetHandler,
                                                    sortBy = config['sort_by'],
                                                    extInfo = config['show_ext'])
                self.statL.append(packageStat)
        if not self.statL:
            print "# no munin config found in environment vars"

    def execute(self):
        self.args = self.argParser.parse_args(self.commandLineArgs)
        self.executionMatrix[self.args.command]()

    def _cacheIsOutdated(self):
        """
        # interesting files are pkgcache.bin (if it exists (it is deleted after apt-get clean))
        # if a file is intstalled or upgraded, '/var/lib/dpkg/status' is changed
        """
        if os.path.isfile(CACHE_FILE):
            cacheMTime = os.stat(CACHE_FILE).st_mtime
        else:
            # no cachestatus file exist, so it _must_ renewed
            return True
        # List of modify-times of different files
        timeL = []
        packageListsDir = "/var/lib/apt/lists"
        files=os.listdir(packageListsDir)
        packageFileL = [ file for file in files if file.endswith('Packages')]
        for packageFile in packageFileL:
            timeL.append(os.stat(os.path.join(packageListsDir, packageFile)).st_mtime)

        dpkgStatusFile = '/var/lib/dpkg/status'
        if os.path.isfile(dpkgStatusFile):
            timeL.append(os.stat(dpkgStatusFile).st_mtime)
        else:
            raise Exception('DPKG-statusfile %r not found, really strange!!!'%dpkgStatusFile)
        newestFileTimestamp = max(timeL)
        age = newestFileTimestamp - cacheMTime 
        if age > 0:
            return True
        else:
            # if we have made a timetravel, we update until we reached good times
            if time() < newestFileTimestamp:
                return True
            return False

    def _run_with_cache(self):
        """ wrapper around _run with writing to file and stdout
            a better way would be a 'shell' tee as stdout
        """
        # cacheNeedUpdate = False
        # if not self.args.nocache:
        #     # check, whether the cachefile has to be written again
        #     if os.path.isfile(CACHE_FILE):
        #         mtime = os.stat(CACHE_FILE).st_mtime
        #         age = time() - mtime
        #         cacheNeedUpdate = age < 0 or age > CACHE_FILE_MAX_AGE
        #     else:
        #         cacheNeedUpdate = True

        if self._cacheIsOutdated() or self.args.nocache:
            # save stdout 
            stdoutDef = sys.stdout
            try:
                out =  StringIO.StringIO()
                sys.stdout = out
                # run writes now to new sys.stdout
                print "# executed at %r (%r)" %(strftime("%s"), strftime("%c"))
                self._run()
                sys.stdout = stdoutDef
                # print output to stdout
                stdoutDef.write(out.getvalue())
                # print output to CACHE_FILE
                with open(CACHE_FILE,'w') as state:
                    state.write(out.getvalue())
            except IOError as e:
                if e.errno == 2:
                    sys.stderr.write("%s : %s" % (e.msg, CACHE_FILE))
                    # 'No such file or directory'
                    os.makedirs( os.path.dirname(CACHE_FILE) )
                else:
                    print sys.stderr.write("%r : %r" % (e, CACHE_FILE))
            finally:
                # restore stdout
                sys.stdout = stdoutDef
        else:
            with open(CACHE_FILE,'r') as data:
                print data.read()

    def _run(self):
        # p … package
        # do the real work
        for p in apt.installedPackages:
            sourceFile = max(p.current_ver.file_list, key=lambda f: PacketHandler.keyOf(f[0]) )[0]
            for packageStat in self.statL:
                packageStat.addPackage(sourceFile, p)

        # print munin output
        for stat in self.statL:
            stat.printValues()

    def run(self):
        if self.args.nocache:
            self._run()
        else:
            self._run_with_cache()

    def config(self):
        for stat in self.statL:
            stat.printConfig()

    def autoconf(self):
        print 'yes'

    def _argParser(self):
        parser = argparse.ArgumentParser(description="Show some statistics "\
                            "about debian packages installed on system by archive",
                           ) 
        parser.set_defaults(command='run', debug=True, nocache=True)

        parser.add_argument('--nocache', '-n', default=False, action='store_true',
                            help='do not use a cache file')
        helpCommand = """
            config ..... writes munin config
            run ........ munin run (writes values)
            autoconf ... writes 'yes'
        """
        parser.add_argument('command', nargs='?', 
                            choices=['config', 'run', 'autoconf', 'drun'],
                            help='mode munin wants to use. "run" is default' + helpCommand)
        return parser

    def _envParser(self):
        """
            reads environVars from [deb_packages] and generate
            a list of dicts, each dict holds a set of settings made in 
            munin config.
            [ 
              { 'type' = 'installed', 
                'sort_by' = ['label', 'archive'],
                'show_ext' = ['origin', 'site'],
              },
              { 'type' = 'upgraded',
                'sort_by' = ['label', 'archive'],
                'show_ext' = ['origin', 'site'],
              }
            ]
        """
        def configStartDict():
            return { 'type': None,
                     'sort_by': dict(),
                     'show_ext' : dict(),
                   }

        interestingVarNameL = [ var for var in os.environ if var.startswith('graph') ]
        config = defaultdict(configStartDict)
        regex = re.compile(r"graph(?P<graphNumber>\d+)_(?P<res>.*?)_?(?P<optNumber>\d+)?$")
        for var in interestingVarNameL:
            m = re.match(regex, var)
            configPart = config[m.group('graphNumber')]
            if m.group('res') == 'type':
                configPart['type'] = os.getenv(var)
            elif m.group('res') == 'sort_by':
                configPart['sort_by'][m.group('optNumber')] = os.getenv(var)
            elif m.group('res') == 'show_ext':
                configPart['show_ext'][m.group('optNumber')] = os.getenv(var)
            else:
                print >> sys.stderr, "configuration option %r was ignored" % (var)
        # we have now dicts for 'sort_by' and 'show_ext' keys 
        # changing them to lists
        for graphConfig in config.itervalues():
            graphConfig['sort_by'] = [val for key, val in sorted(graphConfig['sort_by'].items())]
            graphConfig['show_ext'] = [val for key, val in sorted(graphConfig['show_ext'].items())]
        # we do not want keynames, they are only needed for sorting environmentvars
        return [val for key, val in sorted(config.items())]

    def _envValidater(self):
        """ takes the munin config and checks for valid configuration,
            raises Exception if something is broken
        """
        for graph in self.envConfig:
            if graph['type'] not in ('installed', 'upgradable'):
                print >> sys.stderr, \
                      "GraphType must be 'installed' or 'upgradable' but not %r"%(graph.type), \
                      graph
                raise EnvironmentConfigBroken("Environment Config broken")
            if not graph['sort_by']:
                print >> sys.stderr, \
                      "Graph must be sorted by anything"
                raise EnvironmentConfigBroken("Environment Config broken")
            # check for valid options for sort_by
            unusableOptions = set(graph['sort_by']) - PackageStat.viewSet 
            if unusableOptions: 
                print >> sys.stderr, \
                      "%r are not valid options for 'sort_by'" % (unusableOptions)
                raise EnvironmentConfigBroken("Environment Config broken")
            # check for valid options for sort_by
            unusableOptions = set(graph['show_ext']) - PackageStat.viewSet 
            if unusableOptions:
                print >> sys.stderr, \
                      "%r are not valid options for 'show_ext'" % (x)
                raise EnvironmentConfigBroken("Environment Config broken")

if __name__=='__main__':
    muninPlugin = Munin()
    muninPlugin.execute()
    # import IPython; IPython.embed()


### The following is the smart_ plugin documentation, intended to be used with munindoc

"""
=head1 NAME

deb_packages - plugin to monitor update resources and pending packages on Debian

=head1 APPLICABLE SYSTEMS

This plugin has checked on Debian - Wheezy and squeeze. If you want to use it
on older installations, tell me whether it works or which errors you had. It
shoud run past python-apt 0.7 and python 2.5.

=head1 DESCRIPTION

With this plugin munin can give you a nice graph and some details where your
packages come from, how old or new your installation is. Furtermore it tells
you how many updates you should have been installed, how many packages are
outdated and where they come from.

You can sort installed or upgradable Packages by 'archive', 'origin', 'site',
'label' and 'component' and even some of them at once.

The script uses caching cause it is quite expensive. It saves the output to a
cachefile and checks on each run, if dpkg-status or downloaded Packagefile have
changed. If one of them has changed, it runs, if not it gives you the cached
version

=head1 INSTALLATION

check out this git repository from

=over 2

    aptitude install python-apt
    git clone git://github.com/munin-monitoring/contrib.git
    cd contrib/plugins/apt/deb_packages
    sudo cp deb_packages.py /etc/munin/plugins/deb_packages
    sudo cp deb_packages.munin-conf /etc/munin/plugin-conf.d/deb_packages

=back

Verify the installation by

=over 2

    sudo munin-run deb_packages

=back


=head1 CONFIGURATION

If you copied deb_packages.munin-conf to plugin-conf.d you have a starting point.

A typical configuration looks like this

=over 2

    [deb_packages]
    # plugin is quite expensive and has to write statistics to cache output
    # so it has to write to plugins.cache
    user munin

    # Packagelists to this size are printed as extra information to munin.extinfo
    env.MAX_LIST_SIZE_EXT_INFO 50

    # Age in seconds an $CACHE_FILE can be. If it is older, the script updates
    # default if not set is 3540 (one hour)
    # at the moment this is not used, the plugin always runs (if munin calls it)
    #
    env.CACHE_FILE_MAX_AGE 3540

    # All these numbers are only for sorting, so you can use env.graph01_sort_by_0
    # and env.graph01_sort_by_2 without using env.graph01_sort_by_1.
    # sort_by values ...
    # possible values are 'label', 'archive', 'origin', 'site', 'component'
    env.graph00_type installed
    env.graph00_sort_by_0 label
    env.graph00_sort_by_1 archive
    env.graph00_show_ext_0 origin
    env.graph00_show_ext_1 site

    env.graph01_type upgradable
    env.graph01_sort_by_0 label
    env.graph01_sort_by_1 archive
    env.graph01_show_ext_0 origin
    env.graph01_show_ext_1 site

=back

You can sort_by one or some of these possible Values


=head1 AUTHOR

unknown

=head1 LICENSE

Default for Munin contributions is GPLv2 (http://www.gnu.org/licenses/gpl-2.0.txt)

=cut


"""
