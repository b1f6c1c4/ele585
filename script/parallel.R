library(dplyr)
library(ggplot2)

tmp <- read.csv(file='./data/report/A.csv', header=FALSE, sep=',');
tmp$method <- factor("A");
colnames(tmp) <- c("balanced", "threads", "ninputs", "time", "method");
data <- tmp;

tmp <- read.csv(file='./data/report/B.csv', header=FALSE, sep=',');
tmp$method <- factor("B");
colnames(tmp) <- c("balanced", "threads", "ninputs", "time", "method");
data <- rbind(data, tmp);

tmp <- read.csv(file='./data/report/C.csv', header=FALSE, sep=',');
tmp$method <- factor("C");
colnames(tmp) <- c("balanced", "threads", "ninputs", "time", "method");
data <- rbind(data, tmp);

tmp <- read.csv(file='./data/report/D.csv', header=FALSE, sep=',');
tmp$method <- factor("D");
colnames(tmp) <- c("balanced", "threads", "ninputs", "time", "method");
data <- rbind(data, tmp);

tmp <- read.csv(file='./data/report/E.csv', header=FALSE, sep=',');
tmp$method <- factor("E");
colnames(tmp) <- c("balanced", "threads", "ninputs", "time", "method");
data <- rbind(data, tmp);
rm(tmp);

data$atime <- data$time / data$ninputs;
data$antime <- data$atime * data$threads;
data$n <- factor(data$ninputs);
data$t <- factor(data$threads);

gdata <- data %>% group_by(method, ninputs, balanced, threads) %>% summarise_at(vars(time, atime, antime), mean);
gdata$n <- factor(gdata$ninputs);
gdata$t <- factor(gdata$threads);

ggplot(data, aes(x=t, y=log10(atime), color=n)) + facet_grid(balanced ~ method) + geom_jitter(width=0.25, height=0)
