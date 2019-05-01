library(tidyr)
library(dplyr)
library(ggplot2)

rawdata <- read.csv(file='data/report.csv', header=FALSE, sep=',');
colnames(rawdata) <- c("total", "cores", "type", "time");
rawdata$t <- factor(rawdata$total / 8);
rawdata$c <- factor(rawdata$cores);
data <- rawdata %>%
    group_by(t, c, type) %>%
    summarise(tpt=mean(60 / 8 * time/total), tptc=mean(60 / 8 * time/(total/cores)));

dtpt <- data %>%
    select(-tptc) %>%
    spread(type, tpt) %>%
    mutate(other = total - communication - computation) %>%
    mutate(speed = 1 / total) %>%
    select(-total);
dtptc <- data %>%
    select(-tpt) %>%
    spread(type, tptc) %>%
    mutate(other = total - communication - computation) %>%
    select(-total);

tmp <- gather(dtpt, type, tpt, communication:other, factor_key=TRUE);
data <- tmp;
tmp <- gather(dtptc, type, tptc, communication:other, factor_key=TRUE);
data$tptc <- tmp$tptc;
data$type <- factor(data$type, levels=c("other", "communication", "computation"));

pdf('data/plot.pdf', 5, 5);

ggplot(dtpt, aes(x=c, y=speed)) +
    geom_col() +
    facet_wrap( ~ t) +
    theme(legend.position = 'bottom') +
    xlab('Number of cores') +
    ylab('Speed (Gi entries per second)') +
    ggtitle('Speed up v.s. input size (Gi entries)');

ggplot(data, aes(x=c, y=tptc)) +
    geom_col(position='stack', aes(fill=type)) +
    facet_wrap( ~ t) +
    theme(legend.position = 'bottom') +
    xlab('Number of cores') +
    ylab('CPU time (seconds * cores) per Gi entries') +
    ggtitle('CPU time break down v.s. input size (Gi entries)');
ggplot(data, aes(x=t, y=tptc)) +
    geom_col(position='stack', aes(fill=type)) +
    facet_wrap( ~ c) +
    theme(legend.position = 'bottom') +
    xlab('Number of input entries (Gi)') +
    ylab('CPU time (seconds * cores) per Gi entries') +
    ggtitle('CPU time break down by number of cores');

dev.off();
