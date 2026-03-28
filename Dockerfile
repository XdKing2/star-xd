FROM node:20-bullseye-slim

WORKDIR /root/star-xd

RUN apt-get update && apt-get install -y \
    build-essential \
    python3 \
    make \
    g++ \
    pkg-config \
    git \
    curl \
    ffmpeg \
    imagemagick \
    libvips-dev \
    libglib2.0-dev \
    libcairo2-dev \
    libpango1.0-dev \
    libjpeg62-turbo-dev \
    libgif-dev \
    librsvg2-dev \
    libwebp-dev \
    libsqlite3-dev \
    libx11-dev \
    libxext-dev \
    libxrender-dev \
    && rm -rf /var/lib/apt/lists/*

COPY package.json package-lock.json ./
RUN npm install --build-from-source --legacy-peer-deps

COPY index.js ./
RUN mkdir -p data session temp assets lib plugins

EXPOSE 5000
CMD ["npm", "run", "start:optimized"]
