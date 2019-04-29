library(ggplot2)

d <- read.csv(file='./data/report.csv', header=FALSE, sep=',');
colnames(d) <- c("total", "cores", "time");
d <- d[order(d$cores), ];
d <- d[order(d$total), ];

ggplot(d, aes(x=log2(cores), y=speed)) +
    geom_point(aes(color=log2(total)));
