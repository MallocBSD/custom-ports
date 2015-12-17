# Created by: mallobsd <mallobsd@foxmail.com>
# $FreeBSD$

PORTNAME=	GSL
PORTVERSION=	0.0.1
CATEGORIES=	devel
PKGNAMEPREFIX=	cpp-

MAINTAINER=	mallobsd@foxmail.com
COMMENT=	C++ Guidelines Support Library implemented by Microsoft

LICENSE=	MIT

USE_GITHUB=	yes
GH_ACCOUNT=	Microsoft
GH_PROJECT=	${PORTNAME}
GH_TAGNAME=	c98e1f3

CXX=		clang++37
INC_BASE=	${LOCALBASE}/include
INC_DIRS=	"${INC_BASE}/c++/v1" "${INC_BASE}"
TEST_PORT=	${PORTSDIR}/devel/unittest++
LIBCPP_DEPENDS=	${INC_BASE}/c++/v1/memory:${PORTSDIR}/devel/libc++

USES=		cmake:outsource compiler:c++14-lang

CMAKE_VERBOSE=	yes
CMAKE_EXPORT=	${STAGEDIR}${PREFIX}/lib/cmake/cppgsl
CMAKE_CONFIG=	${CMAKE_EXPORT}/cppgsl-config.cmake

BUILD_DEPENDS+=	clang++37:${PORTSDIR}/lang/clang37 \
		${LIBCPP_DEPENDS} \
		${INC_BASE}/UnitTest++/UnitTest++.h:${TEST_PORT} \
		${LOCALBASE}/lib/libunittest++.so:${TEST_PORT}
RUN_DEPENDS+=	${LIBCPP_DEPENDS}

post-patch:
	@${REINPLACE_CMD} -e 's|include_directories(|& ${INC_DIRS}|g' \
			${WRKSRC}/CMakeLists.txt

post-install:
	@${CP} -r ${WRKSRC}/include ${STAGEDIR}${PREFIX}/include/${PORTNAME}
	@${MKDIR} ${CMAKE_EXPORT}
	@${PRINTF} '%s\n%s' \
		'SET( CPPGSL_INCLUDE_DIR ${INC_DIRS} CACHE PATH "Include path for GSL")' \
		'SET( CPPGSL_FOUND "TRUE" )' >> ${CMAKE_CONFIG}

.include <bsd.port.mk>
