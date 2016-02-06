# Created by: mallocbsd <mallocbsd@foxmail.com>
# $FreeBSD$

PORTNAME?=	ls
PORTVERSION?=	0.0.1
CATEGORIES=	sysutils
MASTER_SITES=	# none
PKGNAMEPREFIX=	mfc-
DISTFILES=	# none

MAINTAINER=	mallocbsd@foxmail.com
COMMENT=	MFC-ed ${PORTNAME}(1)

LICENSE=	BSD2CLAUSE

LIB_DEPENDS=	libxo.so:${PORTSDIR}/devel/libxo

MAIN_SRC?=	bin/${PORTNAME}
ALL_SRCS?=	${MAIN_SRC} ${EXTRA_SRCS}
MAN_FILES?=	man/man1/${PORTNAME}.1 ${EXTRA_MANS}

WRKSRC_MAIN=	${WRKSRC}/${MAIN_SRC}

PLIST_FILES=	bin/${PORTNAME}
.for MAN_FILE in ${MAN_FILES}
PLIST_FILES+=	${MAN_FILE}.gz
.endfor

CARGS=		-I${LOCALBASE}/include -L${LOCALBASE}/lib

.include <bsd.port.pre.mk>

.if ${OSVERSION} != 1001000 || ${ARCH} != "amd64"
IGNORE?=	this port is ONLY for FreeBSD/amd64 10.1-RELEASE
.endif

.for SRC_DIR in ${ALL_SRCS}
.if !exists(${SRC_BASE}/${SRC_DIR})
IGNORE?=	requires FreeBSD sources with ${SRC_DIR} in ${SRC_BASE}
.endif
.endfor

do-extract:
.for SRC_DIR in ${ALL_SRCS}
	@${MKDIR} ${WRKSRC}/`${DIRNAME} ${SRC_DIR}`
	@${CP} -r ${SRC_BASE}/${SRC_DIR} ${WRKSRC}/${SRC_DIR}
.endfor

do-build:
	@${MAKE} -C ${WRKSRC_MAIN} "CARGS=${CARGS}"

do-install:
	${INSTALL_PROGRAM} ${WRKSRC_MAIN}/${PORTNAME} ${STAGEDIR}${PREFIX}/bin
.for MAN_FILE in ${MAN_FILES}
	${INSTALL_MAN} ${WRKSRC_MAIN}/`${BASENAME} ${MAN_FILE}` \
		${STAGEDIR}${MANPREFIX}/${MAN_FILE}
.endfor

.include <bsd.port.post.mk>
