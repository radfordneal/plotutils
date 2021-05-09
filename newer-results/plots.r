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
  pdf(paste0("./",what,".pdf"),height=4.28,width=5,pointsize=11)
  par(mar=c(4.4,3.3,0.2,0.2),mgp=c(2.0,0.6,0),lab=c(7,7,7),cex.lab=1.5)
  col=c("red","red","blue","blue","black","black")
  lty=c(1,2,1,2,1,2)
  plot(1,type="n",xlab="number of terms",ylab="nanoseconds per term",
       xlim=c(1,7),ylim=c(-2.1,6.5),xaxt="n",yaxt="n")
  abline(h=seq(-1,7),col="gray")
  logy <- log(y,2)
  for (j in 1:ncol(y))
  { lines (1:7, logy[,j], col=col[j], lty=lty[j])
    points (1:7, logy[,j], col=col[j], pch=20)
  }
  mtext (2^((-2):6), side=2, line=0.3, at=(-2):6, cex=0.85)
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


# CREATE PLOTS FOR VARIOUS NEWER SYSTEMS.

plot_key()


# AMD ThreadRipper PRO 3945WX.

cache = c(32*1024, 512*1024, 16*1024*1024)

thinkstation_gcc_10_2_0 <- read_times ("results-thinkstation-gcc-10.2.0")
thinkstation_clang_10_0_0 <- read_times ("results-thinkstation-clang-10.0.0")

thinkstation <- thinkstation_clang_10_0_0

plot_times(thinkstation[,,"sum"],  "thinkstation-sum",  cache/8)
plot_times(thinkstation[,,"norm"], "thinkstation-norm", cache/8)
plot_times(thinkstation[,,"dot"],  "thinkstation-dot",  cache/16)

thinkstation_gcc_10_2_0_no_opt <- 
  read_times ("results-thinkstation-gcc-10.2.0-no-opt")
thinkstation_clang_10_0_0_no_opt <- 
  read_times ("results-thinkstation-clang-10.0.0-no-opt")

thinkstation_no_opt <- thinkstation_clang_10_0_0_no_opt

plot_times(thinkstation_no_opt[,,"sum"],  "thinkstation-no-opt-sum",  cache/8)
plot_times(thinkstation_no_opt[,,"norm"], "thinkstation-no-opt-norm", cache/8)
plot_times(thinkstation_no_opt[,,"dot"],  "thinkstation-no-opt-dot",  cache/16)


# Intel i7-3520M.

cache = c(32*1024, 256*1024)

macbookpro2 <- read_times ("results-macbookpro2-gcc-10.2.0")

plot_times(macbookpro2[,,"sum"],  "macbookpro2-sum",  cache/8)
plot_times(macbookpro2[,,"norm"], "macbookpro2-norm", cache/8)
plot_times(macbookpro2[,,"dot"],  "macbookpro2-dot",  cache/16)

macbookpro2_no_opt <- read_times ("results-macbookpro2-gcc-10.2.0-no-opt")

plot_times(macbookpro2_no_opt[,,"sum"],  "macbookpro2_no_opt-sum",  cache/8)
plot_times(macbookpro2_no_opt[,,"norm"], "macbookpro2_no_opt-norm", cache/8)
plot_times(macbookpro2_no_opt[,,"dot"],  "macbookpro2_no_opt-dot",  cache/16)
