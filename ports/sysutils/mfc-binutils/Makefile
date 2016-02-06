# Created by: mallocbsd <mallocbsd@foxmail.com>
# $FreeBSD$

PORTNAME=	binutils
PORTVERSION=	0.0.1
CATEGORIES=	sysutils
PKGNAMEPREFIX=	mfc-

MAINTAINER=	mallocbsd@foxmail.com
COMMENT=	MFC-ed ${PORTNAME}(1)

LICENSE=	BSD2CLAUSE

USES=		metaport

BINUTILS=	df iscsictl ls procstat ps savecore w wc

.for BINUTIL in ${BINUTILS}
RUN_DEPENDS+=	${LOCALBASE}/bin/${BINUTIL}:${PORTSDIR}/sysutils/mfc-${BINUTIL}
.endfor

.include <bsd.port.mk>
