FROM node:18-bullseye-slim

WORKDIR /root/star-xd

RUN apt-get update && apt-get install -y \
    ffmpeg \
    imagemagick \
    git \
    python3 \
    make \
    g++ \
    && rm -rf /var/lib/apt/lists/*

COPY package.json package-lock.json ./
RUN npm install --build-from-source

COPY index.js ./

RUN mkdir -p data session temp

EXPOSE 5000

CMD ["node", "index.js"]
