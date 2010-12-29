## Docbook processing - very simplistic at the moment

DB_URI = http://docbook.sourceforge.net/release/xsl/current

dist_doc_DATA = pattern.txt

dist_html_DATA = reference.html lspipat.png
CLEANFILES = reference.html html_titlepage.xsl
EXTRA_DIST = reference.xml html_custom.xsl html_titlepage.spec.xml

reference.html : reference.xml html_custom.xsl html_titlepage.xsl
	@XSLTPROC@ @XSLT_FLAGS@ -o $@ html_custom.xsl $<

html_titlepage.xsl : html_titlepage.spec.xml
	@XSLTPROC@ @XSLT_FLAGS@ -o $@ $(DB_URI)/template/titlepage.xsl $<
