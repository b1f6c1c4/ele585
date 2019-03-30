library(dplyr)
library(ggplot2)

tmp <- read.csv(file='./data/report/A.csv', header=FALSE, sep=',');
tmp$method <- factor("A");
colnames(tmp) <- c("balance", "threads", "ninputs", "time", "method");
data <- tmp;

tmp <- read.csv(file='./data/report/B.csv', header=FALSE, sep=',');
tmp$method <- factor("B");
colnames(tmp) <- c("balance", "threads", "ninputs", "time", "method");
data <- rbind(data, tmp);

tmp <- read.csv(file='./data/report/C.csv', header=FALSE, sep=',');
tmp$method <- factor("C");
colnames(tmp) <- c("balance", "threads", "ninputs", "time", "method");
data <- rbind(data, tmp);

tmp <- read.csv(file='./data/report/D.csv', header=FALSE, sep=',');
tmp$method <- factor("D");
colnames(tmp) <- c("balance", "threads", "ninputs", "time", "method");
data <- rbind(data, tmp);

tmp <- read.csv(file='./data/report/E.csv', header=FALSE, sep=',');
tmp$method <- factor("E");
colnames(tmp) <- c("balance", "threads", "ninputs", "time", "method");
data <- rbind(data, tmp);

tmp <- read.csv(file='./data/report/F.csv', header=FALSE, sep=',');
tmp$method <- factor("F");
colnames(tmp) <- c("balance", "threads", "ninputs", "time", "method");
data <- rbind(data, tmp);

tmp <- read.csv(file='./data/report/G.csv', header=FALSE, sep=',');
tmp$method <- factor("G");
colnames(tmp) <- c("balance", "threads", "ninputs", "time", "method");
data <- rbind(data, tmp);

tmp <- read.csv(file='./data/report/H.csv', header=FALSE, sep=',');
tmp$method <- factor("H");
colnames(tmp) <- c("balance", "threads", "ninputs", "time", "method");
data <- rbind(data, tmp);

rm(tmp);

data$atime <- data$time / data$ninputs;
data$antime <- data$atime * data$threads;
data$speed <- 1e-6 / data$atime;
data$n <- factor(data$ninputs);
data$t <- factor(data$threads);

rawdata <- data;
data <- rawdata %>%
    group_by(method, ninputs, balance, threads) %>%
    filter((speed > quantile(speed, 0.05)) & (speed < quantile(speed, 0.95)));

makepdf <- function(i) {
    pdf(sprintf("data/report/%d.pdf", i), 10, 12);
    ggplot(subset(data, ninputs==i), aes(x=t, y=speed, fill=balance)) +
    facet_wrap(. ~ method, scales="free_y", ncol=2) +
    geom_violin(scale="width", position="dodge") +
    expand_limits(y=0) +
    xlab("Number of Threads") + ylab("Speed (MOps)") +
    ggtitle(sprintf("Input size = %d", i));
    dev.off();
};

makepdf(128);
makepdf(8192);
makepdf(524288);
makepdf(33554432);
