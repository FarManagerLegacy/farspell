#DOCBOOK_WEBSITE=http://docbook.sourceforge.net/release/website/current
#DOCBOOK_WEBSITE=/usr/share/xml/docbook/custom/website/current
DOCBOOK_WEBSITE=F:/devel/docbook-website-2.6.0

USEJAVA=0
# use HTML Tidy to pretty-print HTML? 1=Yes 0=No
USETIDY=0
# specifies how to call tidy
TIDY=tidy
# specifies options to feed to tidy
TIDYOPTS=-iq -latin1 -mn

ifeq (USEJAVA,1)
# specifies command for calling your XSLT engine
# (this default uses Saxon)
XSLT=java \
	-Djavax.xml.parsers.DocumentBuilderFactory=org.apache.xerces.jaxp.DocumentBuilderFactoryImpl \
	-Djavax.xml.parsers.SAXParserFactory=org.apache.xerces.jaxp.SAXParserFactoryImpl \
	-Djavax.xml.transform.TransformerFactory=com.icl.saxon.TransformerFactoryImpl \
	com.icl.saxon.StyleSheet \
	-x org.apache.xml.resolver.tools.ResolvingXMLReader \
	-y org.apache.xml.resolver.tools.ResolvingXMLReader \
	-r org.apache.xml.resolver.tools.CatalogResolver
else
XSLT=F:/devel/bin.win32/xsltproc.exe
endif

# XMLPARSER specifies command for calling your preferred XML
# parser for validating the DocBook XML sources for your site
#ifeq ($(notdir $(XSLT)),xsltproc)
XMLPARSER=xmllint --valid --noout
#else
#XMLPARSER=java \
#	-Djavax.xml.parsers.DocumentBuilderFactory=org.apache.xerces.jaxp.DocumentBuilderFactoryImpl \
#	-Djavax.xml.parsers.SAXParserFactory=org.apache.xerces.jaxp.SAXParserFactoryImpl \
#	org.apache.xml.resolver.apps.xread
#endif

# the following is empty by default; put any custom DocBook
# stylesheet params you want here; they will be applied globally
# to all HTML transformations from your DocBook XML sources
STYLEOPT=

# name of directory to use if chunking output with "make chunk"
DESTPATH=.


TABSTYLE      =  $(DOCBOOK_WEBSITE)/xsl/tabular.xsl
WEBSTYLE      =  $(DOCBOOK_WEBSITE)/xsl/website.xsl
TABCHUNK      =  $(DOCBOOK_WEBSITE)/xsl/chunk-tabular.xsl
WEBCHUNK      =  $(DOCBOOK_WEBSITE)/xsl/chunk-website.xsl
AUTOLAYOUT    =  $(DOCBOOK_WEBSITE)/xsl/autolayout.xsl 
MAKEDEPENDS   =  $(DOCBOOK_WEBSITE)/xsl/makefile-dep.xsl
MAKETARGETSDB =  $(DOCBOOK_WEBSITE)/xsl/website-targets.xsl

XMLDEPENDS    = autolayout.xml website.database.xml

.PHONY : clean

all:
	$(MAKE) website

STYLESHEET=$(WEBSTYLE)
STYLECHUNK=$(WEBCHUNK)

#ifeq ($(basename $(XSLT)),xsltproc)
#NONTAB_OPT= --stringparam filename-prefix $(NONTABPREFIX)
#else
#NONTAB_OPT= filename-prefix=$(NONTABPREFIX)
#endif
-include depends.nontabular

autolayout.xml: layout.xml
	$(XMLPARSER) $(filter-out $(XMLDEPENDS),$^)
ifeq (USEJAVA,1)
	$(XSLT) $< $(AUTOLAYOUT) use.extensions=1 > $@
else
	$(XSLT) $(AUTOLAYOUT) $< > $@
endif

chunk: autolayout.xml
ifeq (USEJAVA,1)
	$(XSLT) autolayout.xml $(STYLECHUNK) use.extensions=1 output-root=$(DESTPATH)
else
	$(XSLT) --param output-root $(DESTPATH) $(STYLECHUNK) autolayout.xml
endif

%.html: autolayout.xml website.database.xml
	$(XMLPARSER) $(filter-out $(XMLDEPENDS),$^)
ifeq (USEJAVA,1)
	$(XSLT) $(filter-out $(XMLDEPENDS),$^) $(STYLESHEET) $(NONTAB_OPT) $(STYLEOPT) use.extensions=1 > $@ 
else
	$(XSLT) $(NONTAB_OPT) $(STYLEOPT) $(STYLESHEET) $(filter-out $(XMLDEPENDS),$^) > $@
endif
ifeq ($(USETIDY),1)
	-$(TIDY) -iq -latin1 -mn $@
endif

# RDDL gets its own rule because we never want to call tidy on it
rddl.html: autolayout.xml
	$(XMLPARSER) $(filter-out $(XMLDEPENDS),$^)
ifeq (USEJAVA,1)
	$(XSLT) $(NONTAB_OPT) $(STYLEOPT) $(STYLESHEET) $(filter-out $(XMLDEPENDS),$^) > $@
else
	$(XSLT) $(filter-out $(XMLDEPENDS),$^) $(STYLESHEET) $(NONTAB_OPT) $(STYLEOPT) use.extensions=1 > $@
endif

depends.tabular depends.nontabular: autolayout.xml
ifeq (USEJAVA,1)
	$(XSLT) $< $(MAKEDEPENDS) depends-file=$@ $(NONTAB_OPT) use.extensions=1 > $@
else
	$(XSLT) --stringparam depends-file $@ $(NONTAB_OPT) $(MAKEDEPENDS) $< > $@
endif

website.database.xml: autolayout.xml
ifeq (USEJAVA,1)
	$(XSLT) $< $(MAKETARGETSDB) use.extensions=1 > $@
else
	$(XSLT) $(MAKETARGETSDB) $< > $@
endif

depends: autolayout.xml website.database.xml depends.nontabular

realclean: clean
	rm -f autolayout.xml depends.tabular website.database.xml
