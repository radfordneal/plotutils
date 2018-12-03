library(Rmpfr)

check <- function (n)
{
  options(digits=17)
  v <- scan(sep=",");
  if (length(v) %% (n+2) != 0) stop("bad data 1")
  m <- matrix(v,ncol=n+2,byrow=TRUE)
  if (!all(is.na(m[,n+2]))) stop("bad data 2")
  for (i in 1:nrow(m))
  { x <- mpfr(m[i,1:n],precBits=2100)
    r <- as.numeric(roundMpfr(sum(x),53))
    if (r != m[i,n+1])
    { cat("Fail:  item",i," correct =",r," listed =",m[i,n+1],"\n")
    }
  }
}

