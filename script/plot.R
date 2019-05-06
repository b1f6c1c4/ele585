library(tidyr)
library(dplyr)
library(ggplot2)

rawdata <- read.csv(file='data/report.csv', header=FALSE, sep=',');
colnames(rawdata) <- c("total", "cores", "type", "time");
rawdata$t <- factor(rawdata$total / 8);
rawdata$c <- factor(rawdata$cores);
data <- rawdata %>%
    group_by(t, c, type) %>%
    summarise(tpt=mean(time / (total / 8)), tptc=mean(time * cores / (total/8)));

dtpt <- data %>%
    select(-tptc) %>%
    spread(type, tpt) %>%
    mutate(other=total - communication - computation) %>%
    mutate(speed=1 / total) %>%
    select(-total);
dtptc <- data %>%
    select(-tpt) %>%
    spread(type, tptc) %>%
    mutate(other=total - communication - computation) %>%
    select(-total);

tmp <- gather(dtpt, type, tpt, communication:other, factor_key=TRUE);
data <- tmp;
tmp <- gather(dtptc, type, tptc, communication:other, factor_key=TRUE);
data$tptc <- tmp$tptc;
data$type <- factor(data$type, levels=c("other", "communication", "computation"));

pdf('data/plot.pdf', 5, 5);

mu <- 4.2e-9 / 60; lambda <- 7e-9 / 60; phi <- 0.86e-9 / 60;
d <- rawdata %>%
    filter(type == 'total') %>%
    mutate(N=total * 1024 * 1024 * 1024 / 8, M=cores) %>%
    mutate(pred=1024*1024*1024/cores*(phi*log2(N)*log2(M)+(lambda-phi)*log2(M)*log2(M)+mu*log2(N)+(lambda-mu)*log2(M))) %>%
    mutate(act=time / (total / 8)) %>%
    group_by(t, c) %>%
    summarise(actual=1/mean(act), prediction=1/mean(pred)) %>%
    gather(type, value, actual:prediction, factor_key=TRUE);

ggplot(d, aes(x=c, y=value, group=type)) +
    geom_line(aes(linetype=type)) +
    scale_y_log10() +
    facet_wrap( ~ t) +
    theme_bw() +
    theme(legend.position='bottom') +
    theme(axis.text.x=element_text(angle=90, hjust=1)) +
    xlab('Number of cores') +
    ylab('Speed (Gi entries per minutes)');

# ggplot(dtpt, aes(x=c, y=speed)) +
#     geom_col() +
#     facet_wrap( ~ t) +
#     theme(legend.position='bottom') +
#     xlab('Number of cores') +
#     ylab('Speed (Gi entries per minutes)') +
#     ggtitle('Speed up v.s. input size (Gi entries)');

ggplot(data, aes(x=c, y=tptc)) +
    geom_col(position='stack', aes(fill=type)) +
    scale_fill_grey() +
    facet_wrap( ~ t) +
    theme_bw() +
    theme(legend.position='bottom') +
    theme(axis.text.x=element_text(angle=90, hjust=1)) +
    xlab('Number of cores') +
    ylab('CPU time (minutes * cores) per Gi entries');

ggplot(data, aes(x=t, y=tptc)) +
    geom_col(position='stack', aes(fill=type)) +
    scale_fill_grey() +
    facet_wrap( ~ c) +
    theme_bw() +
    theme(legend.position='bottom') +
    theme(axis.text.x=element_text(angle=90, hjust=1)) +
    xlab('Number of input entries (Gi)') +
    ylab('CPU time (minutes * cores) per Gi entries');

dev.off();
