# FUNCTIONS FOR READING TIMING DATA AND CREATING PLOTS.

plot_key <- function ()
{
  pdf("key.pdf",height=0.7,width=9.5,pointsize=11)
  par(mar=c(0.35,0.35,0.35,0.35))
  v <- 0.5
  plot(0,xlim=c(0,10.4),ylim=c(-0.1,v+0.1),
       type="n",xaxt="n",yaxt="n",bty="n",xlab="",ylab="")

  lines(c(0,1),c(v,v),type="b",pch=20,col="red",lty=1)
  text(1.2,v,adj=c(0,0.5),cex=1.25,"Small superaccumulator")

  lines(c(0,1),c(0,0),type="b",pch=20,col="red",lty=2)
  text(1.2,0,adj=c(0,0.5),cex=1.25,"Large superaccumulator")

  lines(c(4.1,5.1),c(v,v),type="b",pch=20,col="blue",lty=1)
  text(5.3,v,adj=c(0,0.5),cex=1.25,"iFastSum")

  lines(c(4.1,5.1),c(0,0),type="b",pch=20,col="blue",lty=2)
  text(5.3,0,adj=c(0,0.5),cex=1.25,"OnlineExact")

  lines(c(7,8),c(v,v),type="b",pch=20,col="black",lty=1)
  text(8.2,v,adj=c(0,0.5),cex=1.25,"Simple sum, ordered")

  lines(c(7,8),c(0,0),type="b",pch=20,col="black",lty=2)
  text(8.2,0,adj=c(0,0.5),cex=1.25,"Simple sum, not ordered")

  dev.off()
}

plot_times <- function (y, what, cache)
{
  pdf(paste0("plots/",what,".pdf"),height=4.28,width=5,pointsize=11)
  par(mar=c(4.4,3.3,0.2,0.2),mgp=c(2.0,0.6,0),lab=c(7,7,7),cex.lab=1.5)
  col=c("red","red","blue","blue","black","black")
  lty=c(1,2,1,2,1,2)
  plot(1,type="n",xlab="number of terms",ylab="nanoseconds per term",
       xlim=c(1,7),ylim=c(-1.1,7.5),xaxt="n",yaxt="n")
  abline(h=seq(-1,7),col="gray")
  logy <- log(y,2)
  for (j in 1:ncol(y))
  { lines (1:7, logy[,j], col=col[j], lty=lty[j])
    points (1:7, logy[,j], col=col[j], pch=20)
  }
  mtext (2^((-1):7), side=2, line=0.3, at=(-1):7, cex=0.85)
  mtext (1:7, side=1, line=0.05, at=(1:7)+0.1, cex=0.7)
  mtext (rep(10,7), side=1, line=0.55, at=(1:7)-0.04, cex=0.85)
  abline(col="gray",v=log10(cache))
  dev.off()
}

read_times <- function (file)
{
  c <- pipe (paste (
   'fgrep -v Kahan <', file, '| fgrep -v "Float 128"', '| fgrep ns/term',
   '| sed "s/.*s,//" | sed "s/ns.*//"'
  ))
  d <- scan(c)
  close(c)
  stopifnot(length(d)==98)
  if (file != "results-pi-gcc-4.6.3")
  { d[d<0] <- d[d<0] + 2^32/1e6  # clock wrapped around
  }
  else
  { d[d<0] <- d[d<0] + 2^32/1e5  # did fewer repeats on the pi
  }
  m <- matrix(d,ncol=7)
  n <- matrix(NA,ncol=7,nrow=2)
  m <- rbind(m[1:8,],n,m[9:12,],n,m[13:14,])
  m <- t(m)
  dim(m) <- c(7,6,3)
  dimnames(m) <- list(NULL,c("sacc","lacc","ifast","online","dbl","udbl"),
                           c("sum","norm","dot"))
  m
}


# CREATE PLOTS FOR VARIOUS SYSTEMS.  When more than one compiler was tried,
# select a compiler to use for reach method based on sum performance with
# 1000 terms for sacc and ifast, 10000 terms for dbl and udbl, and 100000
# terms for lacc and online.

plot_key()


# INTEL 64-BIT SYSTEMS.

# Intel Core 2 Duo (T7700)

cache = c(32*1024, 4*1024*1024)

macbookpro <- read_times ("results-macbookpro-clang-3.6.0")

plot_times(macbookpro[,,"sum"],  "macbookpro-sum",  cache/8)
plot_times(macbookpro[,,"norm"], "macbookpro-norm", cache/8)
plot_times(macbookpro[,,"dot"],  "macbookpro-dot",  cache/16)

# Intel Xeon E5462.

cache <- c(32*1024, 6*1024*1024)

macpro_gcc_4.7.3 <- read_times ("results-macpro-gcc-4.7.3")
macpro_clang_3.5 <- read_times ("results-macpro-clang-3.5")

macpro <- macpro_gcc_4.7.3

macpro[,"ifast",] <- macpro_clang_3.5[,"ifast",]
macpro[,"lacc",] <- macpro_clang_3.5[,"lacc",]
macpro[,"online",] <- macpro_clang_3.5[,"online",]

plot_times(macpro[,,"sum"],  "macpro-sum",  cache/8)
plot_times(macpro[,,"norm"], "macpro-norm", cache/8)
plot_times(macpro[,,"dot"],  "macpro-dot",  cache/16)

# Intel X5680

cache <- c(32*1024, 256*1024, 12*1024*1024)

T7500_gcc_4.6.4 <- read_times ("results-dell-T7500-gcc-4.6.4")
T7500_gcc_4.7.3 <- read_times ("results-dell-T7500-gcc-4.7.3")
T7500_gcc_4.8.1 <- read_times ("results-dell-T7500-gcc-4.8.1")
T7500_gcc_4.9.2 <- read_times ("results-dell-T7500-gcc-4.9.2")
T7500_clang_3.4 <- read_times ("results-dell-T7500-clang-3.4")

T7500 <- T7500_gcc_4.6.4
T7500[,"ifast",] <- T7500_clang_3.4[,"ifast",]
T7500[,"dbl",] <- T7500_clang_3.4[,"dbl",]
T7500[,"udbl",] <- T7500_gcc_4.8.1[,"udbl",]
T7500[,"lacc",] <- T7500_gcc_4.7.3[,"lacc",]
T7500[,"online",] <- T7500_clang_3.4[,"online",]

plot_times(T7500[,,"sum"],  "T7500-sum",  cache/8)
plot_times(T7500[,,"norm"], "T7500-norm", cache/8)
plot_times(T7500[,,"dot"],  "T7500-dot",  cache/16)

# Intel E3-1225

cache <- c(32*1024, 256*1024, 6*1024*1024)

T1600_gcc_4.9.2 <- read_times ("results-dell-T1600-gcc-4.9.2")
T1600_clang_3.5.0 <- read_times ("results-dell-T1600-clang-3.5.0")

T1600 <- T1600_gcc_4.9.2
T1600[,"lacc",] <- T1600_clang_3.5.0[,"lacc",]
T1600[,"online",] <- T1600_clang_3.5.0[,"online",]

plot_times(T1600[,,"sum"],  "T1600-sum",  cache/8)
plot_times(T1600[,,"norm"], "T1600-norm", cache/8)
plot_times(T1600[,,"dot"],  "T1600-dot",  cache/16)

# Intel E3-1230v2

cache <- c(32*1024, 256*1024, 8*1024*1024)

comps <- read_times ("results-comps-gcc-4.8.2")

plot_times(comps[,,"sum"],  "comps-sum",  cache/8)
plot_times(comps[,,"norm"], "comps-norm", cache/8)
plot_times(comps[,,"dot"],  "comps-dot",  cache/16)

# Intel Celeron 1019Y.

cache <- c(32*1024, 246*1024, 2*1024*1024)

aspireV5_gcc_4.8.2 <- read_times ("results-aspireV5-gcc-4.8.2")
aspireV5_clang_3.5.0 <- read_times ("results-aspireV5-clang-3.5.0")

aspireV5 <- aspireV5_gcc_4.8.2
aspireV5[,"ifast",] <- aspireV5_clang_3.5.0[,"ifast",]
aspireV5[,"dbl",] <- aspireV5_clang_3.5.0[,"dbl",]
aspireV5[,"udbl",] <- aspireV5_clang_3.5.0[,"udbl",]
aspireV5[,"online",] <- aspireV5_clang_3.5.0[,"online",]

plot_times(aspireV5[,,"sum"],  "aspireV5-sum",  cache/8)
plot_times(aspireV5[,,"norm"], "aspireV5-norm", cache/8)
plot_times(aspireV5[,,"dot"],  "aspireV5-dot",  cache/16)


# AMD 64-BIT SYSTEMS.

# AMD Opteron 6348

cache = c(16*1024, 2*1024*1024, 6*1024*1024)

cdf <- read_times ("results-cdf-gcc-4.6.3")

plot_times(cdf[,,"sum"],  "cdf-sum",  cache/8)
plot_times(cdf[,,"norm"], "cdf-norm", cache/8)
plot_times(cdf[,,"dot"],  "cdf-dot",  cache/16)

# AMD E1-2500

cache <- c(32*1024, 1024*1024)

gateway_gcc_4.8.2 <- read_times ("results-gateway-gcc-4.8.2")
gateway_clang_3.5.0 <- read_times ("results-gateway-clang-3.5.0")

gateway <- gateway_gcc_4.8.2
gateway[,"ifast",] <- gateway_clang_3.5.0[,"ifast",]
gateway[,"lacc",] <- gateway_clang_3.5.0[,"lacc",]
gateway[,"online",] <- gateway_clang_3.5.0[,"online",]

plot_times(gateway[,,"sum"],  "gateway-sum",  cache/8)
plot_times(gateway[,,"norm"], "gateway-norm", cache/8)
plot_times(gateway[,,"dot"],  "gateway-dot",  cache/16)


# INTEL 32-BIT SYSTEMS.

# Pentium III

cache <- c(16*1024, 256*1024)

gx200_gcc_4.6.3 <- read_times ("results-dell-gx200-gcc-4.6.3")
gx200_clang_3.4 <- read_times ("results-dell-gx200-clang-3.4")

gx200 <- gx200_gcc_4.6.3
gx200[,"sacc",] <- gx200_clang_3.4[,"sacc",]
gx200[,"online",] <- gx200_clang_3.4[,"online",]

plot_times(gx200[,,"sum"],  "gx200-sum",  cache/8)
plot_times(gx200[,,"norm"], "gx200-norm", cache/8)
plot_times(gx200[,,"dot"],  "gx200-dot",  cache/16)

# Xeon (Foster, Pentium 4 based)

cache = c(8*1024, 256*1024)

price_gcc_4.6.4 <- read_times ("results-dell-530-gcc-4.6.4")
price_gcc_4.7.3 <- read_times ("results-dell-530-gcc-4.7.3")
price_gcc_4.8.1 <- read_times ("results-dell-530-gcc-4.8.1")
price_gcc_4.9.2 <- read_times ("results-dell-530-gcc-4.9.2")

price <- price_gcc_4.6.4
price[,"sacc",] <- price_gcc_4.7.3[,"sacc",]
price[,"ifast",] <- price_gcc_4.8.1[,"ifast",]
price[,"dbl",] <- price_gcc_4.7.3[,"dbl",]
price[,"udbl",] <- price_gcc_4.7.3[,"udbl",]
price[,"lacc",] <- price_gcc_4.7.3[,"lacc",]
price[,"online",] <- price_gcc_4.9.2[,"online",]

plot_times(price[,,"sum"],  "price-sum",  cache/8)
plot_times(price[,,"norm"], "price-norm", cache/8)
plot_times(price[,,"dot"],  "price-dot",  cache/16)

# Pentium 4

cache <- c(8*1024, 512*1024)

carlo <- read_times ("results-dell-350-gcc-4.6.3")

plot_times(carlo[,,"sum"],  "carlo-sum",  cache/8)
plot_times(carlo[,,"norm"], "carlo-norm", cache/8)
plot_times(carlo[,,"dot"],  "carlo-dot",  cache/16)

# Xeon X5355, in 32-bit mode.

cache <- c(32*1024, 4*1024*1024)

apps <- read_times ("results-apps-gcc-4.6.3")

plot_times(apps[,,"sum"],  "apps-sum",  cache/8)
plot_times(apps[,,"norm"], "apps-norm", cache/8)
plot_times(apps[,,"dot"],  "apps-dot",  cache/16)


# ARM 32-BIT SYSTEMS.

# ARM V6 COMPATIBLE FOR RASPBERRY PI.

cache <- c(16*1024, 128*1024)

pi <- read_times ("results-pi-gcc-4.6.3")

plot_times(pi[,,"sum"],  "pi-sum",  cache/8)
plot_times(pi[,,"norm"], "pi-norm", cache/8)
plot_times(pi[,,"dot"],  "pi-dot",  cache/16)

# ARM Cortex-A9.

cache <- c(32*1024, 1024*1024)

cubox <- read_times ("results-cubox-gcc-4.8.4")

plot_times(cubox[,,"sum"],  "cubox-sum",  cache/8)
plot_times(cubox[,,"norm"], "cubox-norm", cache/8)
plot_times(cubox[,,"dot"],  "cubox-dot",  cache/16)


# SPARC 64-BIT SYSTEMS.

# ULTRASPARC III.

cache <- c(64*1024, 8*1024*1024)

V880 <- read_times ("results-sun-V880")

plot_times(V880[,,"sum"],  "V880-sum",  cache/8)
plot_times(V880[,,"norm"], "V880-norm", cache/8)
plot_times(V880[,,"dot"],  "V880-dot",  cache/16)

# SPARC T2 Plus.

cache <- c(8*1024, 4*1024*1024)

T5140 <- read_times ("results-sun-T5140")

plot_times(T5140[,,"sum"],  "T5140-sum",  cache/8)
plot_times(T5140[,,"norm"], "T5140-norm", cache/8)
plot_times(T5140[,,"dot"],  "T5140-dot",  cache/16)
