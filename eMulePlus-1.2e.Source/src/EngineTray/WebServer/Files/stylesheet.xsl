<?xml version="1.0" encoding="utf-8" ?> 
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

<xsl:template match="emule">
<html>
<head>
	<meta http-equiv="content-type" content="text/html; charset=utf-8"/>
</head>
<body>
	<xsl:choose>
	<xsl:when test="@connected != 0">
		<table border="1">
		<tr>
			<td>
				<xsl:apply-templates select="header"/>
			</td>
		</tr>
		<xsl:apply-templates select="welcome"/>
		<xsl:apply-templates select="servers"/>
		<xsl:apply-templates select="files"/>
		<xsl:apply-templates select="shared"/>
		<xsl:apply-templates select="logs"/>
		<xsl:apply-templates select="debug"/>
		</table>
		
		<xsl:choose>
		<xsl:when test="@debug != 0">
			<br/>
			<form method="post" action="/debug">
				<i>[Debug] Xml command:</i>
				<br/>
				<textarea name="debug_cmd" cols="80" rows="3"></textarea>
				<input type="hidden" name="test" value="1"/>
				<input type="submit" value="Submit"/>
			</form>
		</xsl:when>
		</xsl:choose>
		<br/>
		<form method="post" action="/">
			<input type="hidden" name="internal_op" value="exit"/>
			<input type="submit" value="Exit engine"/>
		</form>
	</xsl:when>
	<xsl:otherwise>
		Not connected
		<br/>
		<form method="post" action="/">
			<input type="hidden" name="internal_op" value="connect"/>
			Address:
			<input type="text" name="addr">
				<xsl:attribute name="value">
					<xsl:value-of select="@def_addr"/>
				</xsl:attribute>
			</input>
			Port:
			<input type="text" name="port">
				<xsl:attribute name="value">
					<xsl:value-of select="@def_port"/>
				</xsl:attribute>
			</input>
			<input type="submit" value="Connect"/>
		</form>
		
		<form method="post" action="/">
		<input type="hidden" name="internal_op" value="shutdown"/>
		<input type="submit" value="Exit"/>
	</form>
</xsl:otherwise>
	</xsl:choose>
	
	<xsl:choose>
	<xsl:when test="@refresh != 0">
		<br/>
		<i>Refreshing in <xsl:value-of select="@refresh"/> second(s)...</i>
		<script>
			<xsl:text>
			function pageRefresh()
			{
				window.location.reload();
			}
			setTimeout("pageRefresh()", 
			</xsl:text>
			<xsl:value-of select="@refresh"/>000
			<xsl:text>
			);
			</xsl:text>
		</script>
	</xsl:when>
	</xsl:choose>
</body>
</html>
</xsl:template>

<xsl:template match="header">
	<table>
	<tr>
		<td colspan="10">
			eMule Plus version <xsl:value-of select="@version_text"/>
		</td>
	</tr>
	<tr>
		<td>
			<a href="/servers">Servers</a>
		</td>
		<td>
			<a href="/files">Downloads</a>
		</td>
		<td>
			<a href="/files/uploads">Uploads</a>
		</td>
		<td>
			<a href="/search">Search</a>
		</td>
		<td>
			<a href="/shared">Shared Files</a>
		</td>
		<td>
			<a href="/stats">Statistics</a>
		</td>
		<td>
			<a href="/graphs">Graphs</a>
		</td>
		<td>
			<a href="/logs">Logs</a>
		</td>
		<td>
			<a href="/logs/debug">Logs (Debug)</a>
		</td>
		<td>
			<a href="/logs/all">Logs (All)</a>
		</td>
	</tr>
	</table>
</xsl:template>

<xsl:template match="welcome">
	<tr>
		<td>
			Welcome to new ePlus WebInterface!
		</td>
	</tr>
</xsl:template>

<xsl:template match="logs">
	<tr>
		<td>
			<b>Logs</b><br/>
			<table>
			<xsl:for-each select="log">
				<xsl:sort order="ascending" select="@time"/>
				<tr valign="top">
					<td width="150">
						<b><xsl:value-of select="@time_text"/></b>
					</td>
					<td>
						<xsl:value-of select="@text"/>
					</td>
				</tr>
			</xsl:for-each>
			</table>
		</td>
	</tr>
</xsl:template>

<xsl:template match="shared">
	<tr>
		<td>
			<b>Shared files</b><br/>
			<table>
				<tr>
					<td><b>Filename</b></td>
					<td><b>Size</b></td>
					<td><b>Link</b></td>
				</tr>		
				<xsl:for-each select="file">
				<xsl:sort order="ascending" select="@name"/>
				<tr>
					<td>
						<xsl:value-of select="@name"/>
					</td>
					<td>
						<xsl:value-of select="@size"/>
					</td>
					<td>
						<xsl:value-of select="@link"/>
					</td>
				</tr>
				</xsl:for-each>
			</table>
		</td>
	</tr>
</xsl:template>

</xsl:stylesheet>
