# Created by: mallocbsd <mallocbsd@foxmail.com>
# $FreeBSD$

PORTNAME=	unittest++
PORTVERSION=	1.4
CATEGORIES=	devel

MAINTAINER=	mallocbsd@foxmail.com
COMMENT=	Lightweight unit testing framework for C++

LICENSE=	MIT

USE_GITHUB=	yes
GH_ACCOUNT=	Microsoft
GH_PROJECT=	unittest-cpp
GH_TAGNAME=	dc6b908

USES=		cmake:outsource
USE_LDCONFIG=	yes

CMAKE_EXPORT=	${STAGEDIR}${PREFIX}/lib/cmake/${PORTNAME}
CMAKE_CONFIG=	${CMAKE_EXPORT}/${PORTNAME}-config.cmake

post-patch:
	@${REINPLACE_CMD} -e 's/STATIC/SHARED/' ${WRKSRC}/CMakeLists.txt

post-install:
	@${MV} ${STAGEDIR}${PREFIX}/lib/libUnitTest++.so ${STAGEDIR}${PREFIX}/lib/lib${PORTNAME}.so
	@${MKDIR} ${CMAKE_EXPORT}
	@${PRINTF} '%s\n%s\n%s' \
		'SET( UNITTEST++_LIBRARIES "${PREFIX}/lib/lib${PORTNAME}.so" CACHE FILEPATH "Libraries for ${PORTNAME}")' \
		'SET( UNITTEST++_INCLUDE_DIR "${PREFIX}/include" CACHE PATH "Include path for ${PORTNAME}")' \
		'SET( UNITTEST++_FOUND "TRUE" )' >> ${CMAKE_CONFIG}

.include <bsd.port.mk>
