# $FreeBSD$

PORTNAME=	terra
PORTVERSION=	0.0.20160325
CATEGORIES=	lang

MAINTAINER=	mallocbsd@foxmail.com
COMMENT=	Low-level counterpart to Lua

LICENSE=	MIT

LIB_DEPENDS=	libluajit-5.1.so:lang/luajit
BUILD_DEPENDS=	clang35:lang/clang35 llvm-config35:devel/llvm35

USES=		gmake

USE_LDCONFIG=	yes

USE_GITHUB=	yes
GH_ACCOUNT=	zdevito
GH_PROJECT=	terra
GH_TAGNAME=	332a506

CC=		clang35
CXX=		clang++35

CONFLICTS_INSTALL=	terra-devel

post-patch:
	@${PRINTF} "PREFIX=${STAGEDIR}${PREFIX}\n\
		LUAJIT_PREFIX=${PREFIX}\n\
		LUAJIT_VERSION_EXTRA=.5\n" > ${WRKSRC}/Makefile.inc

.include <bsd.port.mk>
