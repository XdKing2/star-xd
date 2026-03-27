FROM quay.io/mrxdking/star-xd:latest

WORKDIR /root/star-xd
COPY package.json package-lock.json ./
RUN npm install
COPY . .
EXPOSE 5000
CMD ["npm", "run", "start:optimized"]